#include "sys.h"

#include <new>

#include "debug.h"
#include "elf.h"
#include "events.h"
#include "idt.h"
#include "kernel.h"
#include "libk.h"
#include "machine.h"
#include "physmem.h"
#include "semaphore.h"
#include "stdint.h"
#include "unique_ptr.h"
#include "utils.h"
#include "vmm.h"
#include "syscalls.h"

int wrapped_sys_handler(uint32_t syscall_type, uint32_t* interrupt_frame) {
    auto process = active_processes.mine();
    auto sys_args = reinterpret_cast<uint32_t*>(interrupt_frame[3]) + 1;

    switch (syscall_type) {
        case 0: {  // exit(int32_t status)
            if (validate_address(sys_args)) {
                SYS::exit_process(sys_args[0]);
            } else {
                SYS::exit_process(-1);
            }
        }

        case 2: {  // int32_t fork()
            auto new_process = new Process(*process);
            new_process->set_return(0);

            process->child_processes.add_stack(new_process);
            go([new_process] {
                new_process->run();
            });

            return new_process->pid;
        }

        case 7: {  // void shutdown()
            Debug::shutdown();
        }

        case 998: {  // yield
            if (!impl::ready_queue.is_empty()) {
                VMM::vmm_off();
                go([process] {
                    process->run();
                });
                Process::release_core();
            }
            return 0;
        }

        case 999: {  // int join();
            auto child = process->child_processes.remove();
            if (child == nullptr) {
                return -1;
            }

            if (child->exit_code.await_ready()) {
                uint32_t code = child->exit_code.await_resume();
                // TODO memory management for kills
                // delete child;
                return code;
            } else {
                VMM::vmm_off();
                child->exit_code.get([process](uint32_t exit_code) {
                    process->set_return(exit_code);
                    process->run();
                });
                // TODO memory management for kills
                // delete child;
                Process::release_core();
            }
        }

        case 1000: {  // int execve(const char* path, char *const argv[], char *const envp[])
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto path = reinterpret_cast<const char*>(sys_args[0]);
            auto argv = reinterpret_cast<char* const*>(sys_args[1]);
            auto envp = reinterpret_cast<char* const*>(sys_args[2]);

            return SYS::execve(path, argv, envp);
        }

        case 1001: {  // int sem(uint32_t counter)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto process = active_processes.mine();
            if (!process->semaphores.can_alloc()) {
                return -1;
            }

            auto id = process->semaphores.alloc(Shared<Semaphore>::make(sys_args[0]));

            return id;
        }

        case 1002: {  // int up(int semaphore)
            if (!validate_address(sys_args)) {
                return 0;
            }

            auto process = active_processes.mine();
            auto semaphore_pointer = process->semaphores.get(sys_args[0]);
            if (semaphore_pointer == nullptr) {
                return -1;
            }
            auto& semaphore = *semaphore_pointer;
            if(semaphore == nullptr) {
                return -1;
            }
            semaphore->up();
            return 0;
        }

        case 1003: {  // int down(int semaphore)
            if (!validate_address(sys_args)) {
                return 0;
            }

            auto process = active_processes.mine();
            auto semaphore_pointer = process->semaphores.get(sys_args[0]);
            if (semaphore_pointer == nullptr) {
                return -1;
            }
            auto& semaphore = *semaphore_pointer;
            if(semaphore == nullptr) {
                return -1;
            }

            if (semaphore->await_ready()) {
                return 0;
            } else {
                VMM::vmm_off();
                process->set_return(0);
                semaphore->down([process] {
                    process->run();
                });
                Process::release_core();
            }
        }

        case 1007: {  // int sem_close(int semaphore)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto process = active_processes.mine();
            if (process->semaphores.remove(sys_args[0])) {
                return 0;
            } else {
                return -1;
            }
        }

        case 1004: {  // void simple_signal(void (*handler)(int, unsigned int))
            if (!validate_address(sys_args)) {
                return 0;
            }
            if (!validate_address(sys_args[0])) {
                return 0;
            }

            active_processes.mine()->signal_handler = sys_args[0];
            return 0;
        }

        case 1006: {  // int sigreturn()
            auto process = active_processes.mine();
            if (!process->in_signal_handler) {
                return -1;
            }

            process->in_signal_handler = false;
            process->context = process->after_signal_context;
            process->run();
        }

        case 1005: {  // uint32_t simple_mmap(uint32_t addr, uint32_t size, int fd, uint32_t offset)
            // simple_mmap has a different failure code for if the args are in
            // unmapped memory
            process->set_return(0);

            if (!validate_address(sys_args, 4)) {
                return 0;
            }

            uint32_t start = sys_args[0];
            uint32_t size = sys_args[1];
            int32_t file = sys_args[2];

            auto process = active_processes.mine();
            if (file == -1) {
                return process->memory_mapping.map(start, size);
            }

            uint32_t offset = sys_args[3];
            if (offset % PhysMem::FRAME_SIZE != 0) {
                return 0;
            }
            auto fd_pointer = process->file_descriptors.get(file);
            if (fd_pointer == nullptr) {
                return 0;
            }
            auto& fd = *fd_pointer;
            if (fd == nullptr || !fd->supports_offset()) {
                return 0;
            }

            uint32_t file_size = fd->get_length();
            if (offset >= file_size) {
                return process->memory_mapping.map(start, size);
            }

            return process->memory_mapping.map(start, size, fd, offset, file_size);
        }

        case 1008: {  // int simple_munmap(uint32_t address)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto process = active_processes.mine();
            if (process->memory_unmap(sys_args[0])) {
                return 0;
            } else {
                return -1;
            }
        }

        case 1020: {  // void chdir(const char* path)
            auto process = active_processes.mine();

            // since an invalid path causes the CWD to be set to null, the cwd
            // should be moved out of the Process, and will be saved a local
            // this makes it so that if there is an unmapped page of memory in
            // the path, the CWD will be left as null

            auto previous_cwd = process->cwd.release();
            auto previous_cwd_inode = previous_cwd != nullptr ? previous_cwd->number : 0;
            delete previous_cwd;

            if (!validate_address(sys_args)) {
                return -1;
            }

            // ensure that the path is all mapped and in user space memory
            auto path = reinterpret_cast<const char*>(sys_args[0]);
            if (Utils::validate_and_count(path) == -1) {
                return -1;
            }

            if (previous_cwd_inode != 0) {
                process->cwd = UniquePtr(fs->get_node(previous_cwd_inode));
            }
            process->change_cwd(path);

            return 0;
        }

        case 1021: {  // int open(const char* path)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto path = reinterpret_cast<const char*>(sys_args[0]);
            if (Utils::validate_and_count(path) == -1) {
                return -1;
            }

            auto process = active_processes.mine();
            if (!process->file_descriptors.can_alloc()) {
                return -1;
            }

            auto node = process->lookup_path(path);
            if (node == nullptr) {
                return -1;
            }

            return process->file_descriptors.alloc(FileDescriptor::from_node(node));
        }

        case 1022: {  // int close(int fd)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto process = active_processes.mine();
            if (process->file_descriptors.remove(sys_args[0])) {
                return 0;
            } else {
                return -1;
            }
        }

        case 1023: {  // int len(int fd)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto process = active_processes.mine();
            auto fd_pointer = process->file_descriptors.get(sys_args[0]);
            if (fd_pointer == nullptr) {
                return 0;
            }
            auto& fd = *fd_pointer;
            if (fd == nullptr) {
                return -1;
            }

            if (fd->supports_offset()) {
                return fd->get_length();
            } else {
                return -1;
            }
        }

        case 1024: {  // int read(int fd, void* buffer, uint n_bytes)
            if (!validate_address(sys_args, 3)) {
                return -1;
            }

            if (sys_args[2] == 0) {
                return 0;
            }

            // only reading 1 byte at a time for now, but make sure the buffer is mapped
            auto buffer = reinterpret_cast<char*>(sys_args[1]);
            if (!validate_address(buffer)) {
                return -1;
            }
            buffer[0] = buffer[0];

            auto process = active_processes.mine();
            auto fd_pointer = process->file_descriptors.get(sys_args[0]);
            if (fd_pointer == nullptr) {
                return 0;
            }
            auto& fd = *fd_pointer;
            if (fd == nullptr || !fd->is_readable()) {
                return -1;
            }

            return fd->read(buffer, 1, [process](auto schedule) {
                VMM::vmm_off();
                schedule([process](auto n_read, auto write_data) {
                    // TODO clean this up... page dir needs to be on before writing data though
                    vmm_on(process->page_dir);
                    active_processes.mine() = process;
                    write_data();
                    process->set_return(n_read);
                    process->run();
                });
                Process::release_core();
            });
        }

        case 1:
        case 1025: {  // int write(int fd, void* buffer, uint n_bytes)
            if (!validate_address(sys_args, 3)) {
                return -1;
            }

            if (sys_args[2] == 0) {
                return 0;
            }

            // only reading 1 byte at a time for now, but make sure the buffer is mapped
            auto buffer = reinterpret_cast<char*>(sys_args[1]);
            if (!validate_address(buffer)) {
                return -1;
            }
            (void)buffer[0];

            auto process = active_processes.mine();
            auto fd_pointer = process->file_descriptors.get(sys_args[0]);
            if (fd_pointer == nullptr) {
                return 0;
            }
            auto& fd = *fd_pointer;
            if (fd == nullptr || !fd->is_writable()) {
                return -1;
            }

            return fd->write(buffer, 1, [process](auto schedule) {
                schedule([process](auto n_written) {
                    process->set_return(n_written);
                    process->run();
                });
                Process::release_core();
            });
        }

        case 1026: {  // int pipe(int* write_fd, int* read_fd)
            if (!validate_address(sys_args, 2)) {
                return -1;
            }

            auto descriptors = reinterpret_cast<uint32_t**>(sys_args);
            if (!validate_address(descriptors[0]) || !validate_address(descriptors[1])) {
                return -1;
            }

            // ensure that they are mapped, if they aren't these will fault and return -1
            sys_args[0] = sys_args[0];
            sys_args[1] = sys_args[1];

            auto process = active_processes.mine();
            if (!process->file_descriptors.can_alloc(2)) {
                return -1;
            }

            auto buffer = Shared<PipeBuffer>::make(100);
            *descriptors[0] = process->file_descriptors.alloc(FileDescriptor::from_bounded_buffer(buffer, false));
            *descriptors[1] = process->file_descriptors.alloc(FileDescriptor::from_bounded_buffer(buffer, true));

            return 0;
        }

        case 1028: {  // int dup(int fd)
            if (!validate_address(sys_args)) {
                return -1;
            }

            auto process = active_processes.mine();
            if (!process->file_descriptors.can_alloc()) {
                return -1;
            }

            auto fd_pointer = process->file_descriptors.get(sys_args[0]);
            if (fd_pointer == nullptr) {
                return 0;
            }
            auto& fd = *fd_pointer;
            if (fd == nullptr) {
                return -1;
            }

            return process->file_descriptors.alloc(fd);
        }

        case 1027: {  // int kill(uint code)
            if (!validate_address(sys_args)) {
                return -1;
            }

            // TODO return -1 if child exited
            auto child = process->child_processes.get_front();
            if (child == nullptr || child->is_killed) {
                return -1;
            }

            child->is_killed = true;
            child->kill_argument = sys_args[0];
            return 0;
        }

        case 1029: { // int lseek(int fd, int offset, int whence)
            if (!validate_address(sys_args, 3)) {
                return -1;
            }
            return lseek(sys_args[0], sys_args[1], sys_args[2]);
        }

        case 1050: { // int isatty(int fd)
            if (!validate_address(sys_args)) {
                return -1;
            }
            return isatty(sys_args[0]);
        }

        default:
            Debug::panic("syscall %d\n", syscall_type);
    }

    return 0;
}

extern "C" int sysHandler(uint32_t syscall_type, uint32_t* frame) {
    auto process = active_processes.mine();
    uint32_t* interrupt_frame = frame + 8;
    uint32_t* register_frame = frame;

    process->save_state(interrupt_frame, register_frame);

    // default error code return value, would be used in the page fault handler
    // if the syscall results in a segfault
    process->set_return(-1);

    process->set_return(wrapped_sys_handler(syscall_type, interrupt_frame));
    process->run();

    return 0;
}

void SYS::init(void) {
    IDT::trap(48, (uint32_t)sysHandler_, 3);
}

int32_t SYS::execve(const char* pathname, char* const argv[], char* const envp[], bool is_initial) {
    Process* new_process = nullptr;

    {
        if (Utils::validate_and_count(pathname, is_initial) == -1) {
            return -1;
        }

        int32_t n_args = Utils::validate_and_count(argv, is_initial);
        if (n_args == -1) {
            return -1;
        }

        int32_t n_envs;
        if (is_initial) {
            // init will be launched with envp == nullptr, do not try to validate
            n_envs = 0;
        } else {
            n_envs = Utils::validate_and_count(envp, is_initial);
            if (n_envs == -1) {
                return -1;
            }
        }

        uint32_t total_length = 0;
        for (int32_t i = 0; i < n_envs; i++) {
            int32_t var_length = Utils::validate_and_count(envp[i], is_initial);
            if (var_length == -1) {
                return -1;
            }
            total_length += var_length + 1;
        }

        for (int32_t i = 0; i < n_args; i++) {
            int32_t arg_length = Utils::validate_and_count(argv[i], is_initial);
            if (arg_length == -1) {
                return -1;
            }
            total_length += arg_length + 1;
        }

        // it might be more efficient sometimes to validate args after testing
        // the file, but if there were an unmapped memory access, the syscall
        // would get terminated and memory would leak
        // doing args before any allocations makes it so there will not be leaks

        // find and validate the ELF file
        auto previous_process = active_processes.mine();
        auto file = UniquePtr(previous_process == nullptr
                                  ? find_fs_node(pathname)
                                  : previous_process->lookup_path(pathname));
        if (file == nullptr || !file->is_file()) {
            return -1;
        }

        auto elf_info = ELF::validate(file);
        if (elf_info.is_invalid()) {
            return -1;
        }

        // this will auto free the ELF data when it goes out of scope
        auto smart_elf = UniquePtr<ElfLoadList::ElfLoad, ElfLoadList::ElfLoad[]>(elf_info.loads);

        uint32_t new_directory = VMM::new_directory();
        uint32_t initial_esp = VMM::set_up_stack(new_directory, total_length, argv, n_args, envp, n_envs);
        vmm_on(new_directory);

        if (previous_process != nullptr) {
            previous_process->reset(elf_info.entry_point, initial_esp, new_directory);
            new_process = previous_process;
        } else {
            new_process = new Process(elf_info.entry_point, initial_esp, new_directory);
        }

        // map user stack
        new_process->memory_mapping.map(USER_STACK_END, USER_STACK_SIZE);

        // map elf sections
        auto fd = FileDescriptor::from_node(file.release());
        for (uint32_t i = 0; i < elf_info.load_count; i++) {
            auto& load = elf_info.loads[i];
            ASSERT(new_process->memory_mapping.map(
                       load.load_address, PhysMem::frameup(load.memory_size),
                       fd, load.offset, load.file_size) > 0);
        }
    }

    new_process->run();

    return 0;
}

void SYS::exit_process(uint32_t exit_code) {
    auto process = active_processes.mine();
    process->exit_code.set(exit_code);

    // TODO properly kill and free memory
    // delete process;
    VMM::vmm_off();
    VMM::free_directory(process->page_dir);
    active_processes.mine() = nullptr;

    event_loop();
}