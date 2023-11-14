#pragma once

#include "descriptor_list.h"
#include "elf.h"
#include "file_descriptor.h"
#include "future.h"
#include "memory_mapping.h"
#include "queue.h"
#include "semaphore.h"
#include "shared.h"
#include "stdint.h"
#include "unique_ptr.h"

struct ProcessContext {
    // regs from interrupt
    uint32_t eip;
    uint32_t eflags{0x2 | 0x200};  // reserved=1, interrupt enable
    uint32_t esp;

    // regs from pusha
    uint32_t edi{0};
    uint32_t esi{0};
    uint32_t ebp{0};
    uint32_t ebx{0};
    uint32_t edx{0};
    uint32_t ecx{0};
    uint32_t eax{0};

    ProcessContext() {}
    ProcessContext(uint32_t entry_point, uint32_t initial_esp) : eip{entry_point}, esp{initial_esp} {}
};

struct Process {
    uint32_t pid;
    uint32_t page_dir;
    UniquePtr<Node> cwd;
    ProcessContext context;
    Future<uint32_t> exit_code;
    MemoryMapping memory_mapping;
    DescriptorList<Shared<FileDescriptor>, 10> file_descriptors;
    DescriptorList<Shared<Semaphore>, 100> semaphores;
    Queue<Process, NoLock> child_processes;
    Process* next;  // for queue

    // signals
    bool in_signal_handler{false};
    uint32_t signal_handler{0};
    ProcessContext after_signal_context;

    // kills
    bool is_killed{false};
    uint32_t kill_argument;

    Process(uint32_t entry_point, uint32_t initial_esp, uint32_t page_dir);
    Process(Process& other);
    ~Process();

    void run();
    void run_signal_handler(uint32_t code, uint32_t argument);
    void save_state(uint32_t* interrupt_frame, uint32_t* register_frame);
    void set_return(uint32_t return_value);
    void reset(uint32_t new_entry_point, uint32_t new_esp, uint32_t new_page_dir);

    Node* lookup_path(const char* path);
    void change_cwd(const char* path);

    bool memory_unmap(uint32_t address);

    static void release_core();
};