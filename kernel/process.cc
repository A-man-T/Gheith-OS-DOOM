#include "process.h"

#include "kernel.h"
#include "machine.h"
#include "sys.h"
#include "vmm.h"

static Atomic<uint32_t> pid_increment{0};

Process::Process(uint32_t entry_point, uint32_t initial_esp, uint32_t page_dir)
    : pid{pid_increment.add_fetch(1)},
      page_dir{page_dir},
      cwd{new Node(fs->root)},
      context{entry_point, initial_esp} {
    // create stdin, stdout, stderr
    file_descriptors.set(0, FileDescriptor::from_terminal(0));
    file_descriptors.set(1, FileDescriptor::from_terminal(1));
    file_descriptors.set(2, FileDescriptor::from_terminal(2));
}

Process::Process(Process& other)
    : pid{pid_increment.add_fetch(1)},
      page_dir{VMM::clone_directory(other.page_dir)},
      cwd{other.cwd != nullptr ? new Node(other.cwd) : nullptr},
      context{other.context},
      exit_code{},
      memory_mapping{other.memory_mapping},
      file_descriptors{other.file_descriptors},
      semaphores{other.semaphores},
      child_processes{},
      in_signal_handler{other.in_signal_handler},
      signal_handler{other.signal_handler},
      after_signal_context{other.after_signal_context} {}

Process::~Process() {
    VMM::vmm_off();
    VMM::free_directory(page_dir);
    // child_processes.free_all_items();
    active_processes.mine() = nullptr;
}

void Process::run() {
    if (kernel_interrupted.mine()) {
        VMM::vmm_off();
        go([this] {
            run();
        });
        release_core();
    } else {
        if ((getCR3() & 0xFFFF'F000) != page_dir) {
            vmm_on(page_dir);
        }
        active_processes.mine() = this;

        if (is_killed) {
            is_killed = false;
            run_signal_handler(2, kill_argument);
            SYS::exit_process(kill_argument);
        } else {
            start_process(&context);
        }
    }
}

constexpr uint8_t sigreturn_bytecode[] = {
    0xb8, 0xee, 0x03, 0x00, 0x00,  // mov $1006, %eax
    0xcd, 0x30                     // int $48
};
void Process::run_signal_handler(uint32_t code, uint32_t argument) {
    if (signal_handler == 0 || in_signal_handler) {
        return;
    }

    after_signal_context = context;
    in_signal_handler = true;

    context.esp -= 128 + 4 * 3 + sizeof(sigreturn_bytecode);
    context.esp &= 0xFFFF'FFF0;

    auto user_stack = (uint32_t*)context.esp;
    if (!validate_address(user_stack, 3)) {
        return;
    }

    user_stack[0] = (uint32_t)(user_stack + 3);
    user_stack[1] = code;
    user_stack[2] = argument;
    memcpy(user_stack + 3, sigreturn_bytecode, sizeof(sigreturn_bytecode));

    context.eip = signal_handler;
    run();
}

void Process::save_state(uint32_t* interrupt_frame, uint32_t* register_frame) {
    context.eip = interrupt_frame[0];
    context.eflags = interrupt_frame[2];
    context.esp = interrupt_frame[3];

    context.edi = register_frame[0];
    context.esi = register_frame[1];
    context.ebp = register_frame[2];
    context.ebx = register_frame[4];  // skip esp
    context.edx = register_frame[5];
    context.ecx = register_frame[6];
    context.eax = register_frame[7];
}

void Process::set_return(uint32_t return_value) {
    context.eax = return_value;
}

void Process::reset(uint32_t new_entry_point, uint32_t new_esp, uint32_t new_page_dir) {
    VMM::free_directory(page_dir);
    memory_mapping.clear();
    context.eip = new_entry_point;
    context.esp = new_esp;
    page_dir = new_page_dir;
    signal_handler = 0;
    in_signal_handler = false;
}

Node* Process::lookup_path(const char* path) {
    return find_fs_node(path, cwd);
}

void Process::change_cwd(const char* path) {
    auto node = UniquePtr(lookup_path(path));
    if (node == nullptr || !node->is_dir()) {
        cwd = nullptr;
    } else {
        cwd = node;
    }
}

bool Process::memory_unmap(uint32_t address) {
    auto result = memory_mapping.unmap(address);
    if (result.size == 0) {
        return false;
    }

    VMM::unmap_pages(result.start, result.size, page_dir);

    return true;
}

void Process::release_core() {
    active_processes.mine() = nullptr;
    event_loop();
}