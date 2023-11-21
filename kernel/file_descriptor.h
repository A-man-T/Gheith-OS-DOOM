#pragma once

#include <new>

#include "atomic.h"
#include "bounded_buffer.h"
#include "ext2.h"
#include "libk.h"
#include "shared.h"
#include "stdint.h"
#include "vmm.h"

struct CharWrapper {
    char data;
    bool eof;
};

struct PipeBuffer {
    BoundedBuffer<CharWrapper> buffer;
    bool has_closed_all_writers = false;
    Atomic<int> bytesInBuffer{0};
    PipeBuffer(int size) : buffer(size){
    }
};

class FileDescriptor {
   private:
    enum class Type {
        File,
        Folder,
        StdIn,
        StdOut,
        StdErr,
        PipeRead,
        PipeWrite
    };

   
    struct Data {
        enum class Tag {
            None,
            File,
            Buffer
        };

        union Union {
            Node* file;
            Shared<PipeBuffer> buffer;

            Union() {}
            ~Union() {}
        };

        Tag tag;
        Union data;

        Data() : tag{Tag::None} {}

        ~Data() {
            reset();
        }

        void set_file(Node* file) {
            reset();
            tag = Tag::File;
            data.file = file;
        }

        Node* get_file() {
            ASSERT(tag == Tag::File);
            return data.file;
        }

        void reset_file() {
            ASSERT(tag == Tag::File);
            delete data.file;
            data.file = nullptr;
        }

        template <typename... Args>
        void construct_buffer(Args... args) {
            reset();
            tag = Tag::Buffer;
            new (&data.buffer) Shared<PipeBuffer>(args...);
        }

        Shared<PipeBuffer> get_buffer() {
            ASSERT(tag == Tag::Buffer);
            return data.buffer;
        }

        void destruct_buffer() {
            ASSERT(tag == Tag::Buffer);
            data.buffer.~Shared();
        }

        void reset() {
            switch (tag) {
                case Tag::None:
                    break;

                case Tag::File:
                    reset_file();
                    break;

                case Tag::Buffer:
                    destruct_buffer();
                    break;
            }
            tag = Tag::None;
        }
    };

    Type type;
    Atomic<uint32_t> offset{0};
    Data data;

   public:
    FileDescriptor(Type type);
    FileDescriptor(FileDescriptor&& other);
    ~FileDescriptor();

    bool is_readable();
    bool is_writable();

    template <typename Scheduler>
    uint32_t read(char* buffer, uint32_t size, Scheduler schedule, uint32_t position = 0xFFFF'FFFF) {
        ASSERT(is_readable());

        switch (type) {
            case Type::File: {
                uint32_t file_size = data.get_file()->size_in_bytes();
                if (position == 0xFFFF'FFFF) {
                    while (true) {
                        uint32_t loaded_offset = offset.get();
                        uint32_t updated_size = K::min(loaded_offset + size, file_size) - loaded_offset;

                        if (offset.cmp_exchange(loaded_offset, loaded_offset + updated_size)) {
                            position = loaded_offset;
                            size = updated_size;
                            break;
                        }
                    }
                }

                size = K::min(position + size, file_size) - position;
                data.get_file()->read_all(position, size, buffer);

                return size;
            }

            case Type::PipeRead: {
                ASSERT(position == 0xFFFF'FFFF);
                if (size == 0) {
                    return 0;
                }
                if (data.get_buffer()->has_closed_all_writers && data.get_buffer()->bytesInBuffer.get() <= 0) {
                    return 0;
                }
                data.get_buffer()->bytesInBuffer.add_fetch(-1);
                schedule([buffer, bounded_buffer = data.get_buffer()](auto continuation) {
                    bounded_buffer->buffer.get([buffer, continuation, bounded_buffer](auto data) {
                        continuation((data.eof) ? 0 : 1, [buffer, data, bounded_buffer] {
                            if (!data.eof) {
                                *buffer = data.data;
                            } else {
                                bounded_buffer->buffer.put(data, []{});
                            }
                        });
                    });
                });
                return -1;
            }

            default:
                MISSING();
        }

        return -1;
    }

    template <typename Scheduler>
    uint32_t write(const char* buffer, uint32_t size, Scheduler schedule) {
        ASSERT(is_writable());

        switch (type) {
            case Type::StdOut:
            case Type::StdErr: {
                for (uint32_t i = 0; i < size; i++) {
                    Debug::printf("%c", buffer[i]);
                }
                return size;
            }

            case Type::PipeWrite: {
                if (size == 0) {
                    return 0;
                }
                data.get_buffer()->bytesInBuffer.add_fetch(1);
                CharWrapper valToPut = {*buffer, false};
                schedule([valToPut, bounded_buffer = data.get_buffer()](auto continuation) {
                    bounded_buffer->buffer.put(valToPut, [continuation] {
                        VMM::vmm_off();
                        continuation(1);
                    });
                });
                return -1;
            }

            default:
                MISSING();
        }

        return -1;
    }

    bool supports_offset();
    bool is_tty();
    uint32_t get_length();
    uint32_t get_offset();
    void set_offset(uint32_t new_offset);

    static Shared<FileDescriptor> from_node(Node* node);
    static Shared<FileDescriptor> from_terminal(uint32_t code);
    static Shared<FileDescriptor> from_bounded_buffer(Shared<PipeBuffer> buffer, bool is_read_end);

   private:
    bool uses_file();
    bool uses_buffer();
};
