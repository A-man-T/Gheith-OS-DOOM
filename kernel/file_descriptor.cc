#include "file_descriptor.h"

FileDescriptor::FileDescriptor(FileDescriptor::Type type)
    : type{type} {}

FileDescriptor::FileDescriptor(FileDescriptor&& other)
    : type{other.type}, offset{other.offset} {
    if (uses_file()) {
        data.set_file(other.data.get_file());
        other.data.reset_file();
    } else if (uses_buffer()) {
        data.construct_buffer(other.data.get_buffer());
        other.data.destruct_buffer();
    }
}

bool FileDescriptor::is_readable() {
    return type == Type::File || type == Type::PipeRead;
}

bool FileDescriptor::is_writable() {
    return type == Type::StdOut || type == Type::StdErr || type == Type::PipeWrite;
}

bool FileDescriptor::supports_offset() {
    return type == Type::File;
}

uint32_t FileDescriptor::get_length() {
    ASSERT(supports_offset());
    return data.get_file()->size_in_bytes();
}

uint32_t FileDescriptor::get_offset() {
    ASSERT(supports_offset());
    return offset;
}

void FileDescriptor::set_offset(uint32_t new_offset) {
    ASSERT(supports_offset());
    offset = new_offset;
}

Shared<FileDescriptor> FileDescriptor::from_node(Node* node) {
    ASSERT(node->is_file() || node->is_dir());

    auto descriptor = Shared<FileDescriptor>::make(
        node->is_file() ? Type::File : Type::Folder);
    descriptor->data.set_file(node);

    return descriptor;
}

Shared<FileDescriptor> FileDescriptor::from_terminal(uint32_t code) {
    ASSERT(code < 3);

    auto descriptor = Shared<FileDescriptor>::make(
        code == 0
            ? Type::StdIn
            : (code == 1 ? Type::StdOut : Type::StdErr));

    return descriptor;
}

Shared<FileDescriptor> FileDescriptor::from_bounded_buffer(Shared<BoundedBuffer<char>> buffer, bool is_read_end) {
    auto descriptor = Shared<FileDescriptor>::make(
        is_read_end ? Type::PipeRead : Type::PipeWrite);
    descriptor->data.construct_buffer(buffer);

    return descriptor;
}

bool FileDescriptor::uses_file() {
    return type == Type::File || type == Type::Folder;
}

bool FileDescriptor::uses_buffer() {
    return type == Type::PipeRead || type == Type::PipeWrite;
}