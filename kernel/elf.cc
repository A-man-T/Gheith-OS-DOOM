#include "elf.h"

#include "debug.h"
#include "kernel.h"

ElfLoadList ELF::validate(Node* file) {
    uint32_t size = file->size_in_bytes();

    // check to make sure the ELF header is readable
    if (size < sizeof(ElfHeader)) {
        return {nullptr};
    }

    ElfHeader header;
    file->read(0, header);

    // check magic number
    if (header.magic0 != 0x7f || header.magic1 != 'E' || header.magic2 != 'L' || header.magic3 != 'F') {
        return {nullptr};
    }

    // check encodings/version/system
    if (header.cls != 1 || header.encoding != 1 || header.header_version != 1 || header.abi != 0) {
        return {nullptr};
    }

    // check types
    if (header.type != 2 || header.machine != 3 || header.version != 1) {
        return {nullptr};
    }

    if (!validate_address(header.entry)) {
        return {nullptr};
    }

    // check to make sure all the ELF program headers are readable
    if (!check_bounds(header.phoff, header.phnum, sizeof(ProgramHeader), 0, size + 1)) {
        return {nullptr};
    }

    ElfLoadList list{file, header.entry, 0, new ElfLoadList::ElfLoad[header.phnum]};

    ProgramHeader program_header;
    bool entry_point_found = false;
    for (uint16_t i = 0; i < header.phnum; i++) {
        file->read(header.phoff + i * header.phentsize, program_header);

        // only handle LOAD sections with size > 0
        if (program_header.type != 1 || program_header.memsz == 0) {
            continue;
        }

        if (!validate_address(program_header.vaddr, program_header.memsz)) {
            delete[] list.loads;
            return {nullptr};
        }

        if (program_header.offset + program_header.filesz > size) {
            delete[] list.loads;
            return {nullptr};
        }

        // TODO validate nonoverlapping sections and page alignment?

        if (header.entry >= program_header.vaddr && header.entry < program_header.vaddr + program_header.memsz) {
            entry_point_found = true;
        }

        list.loads[list.load_count++] = {
            program_header.offset, program_header.filesz,
            program_header.vaddr, program_header.memsz};
    }

    if (!entry_point_found) {
        delete[] list.loads;
        return {nullptr};
    }

    return list;
}

uint32_t ELF::load(ElfLoadList data) {
    for (uint32_t i = 0; i < data.load_count; i++) {
        auto header = data.loads[i];
        char* load_at = reinterpret_cast<char*>(header.load_address);
        if (header.file_size > 0) {
            data.file->read_all(header.offset, header.file_size, load_at);
        }
    }

    return data.entry_point;
}

uint32_t ELF::load(Node* file) {
    auto data = ELF::validate(file);
    if (data.is_invalid()) return 0;
    uint32_t entry = ELF::load(data);
    delete[] data.loads;
    return entry;
}