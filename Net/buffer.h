//
// Created by rui836662398 on 2021/2/27.
//

#ifndef MYWEBSERVER_BUFFER_H
#define MYWEBSERVER_BUFFER_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <vector>

class Buffer {
   public:
    static const char kCRLF[];
    static const size_t kDefaultSize = 1024;
    static const size_t kPrependSize = 8;

    explicit Buffer(size_t size = kDefaultSize)
        : buffer_(kDefaultSize + kPrependSize),
          read_index_(kPrependSize),
          write_index_(kPrependSize) {}

    size_t ReadableBytes() const { return write_index_ - read_index_; }
    size_t WritableBytes() const { return buffer_.size() - write_index_; }
    size_t PrePendedBytes() const { return read_index_; }

    char* ReadData() { return begin() + read_index_; }
    const char* ReadData() const { return begin() + read_index_; }
    char* WriteData() { return begin() + write_index_; }
    const char* WriteData() const { return begin() + write_index_; }

    void Reset() {
        read_index_ = kPrependSize;
        write_index_ = kPrependSize;
    }
    // skip len bytes of readable buffer
    void Skip(size_t len) {
        assert(len <= ReadableBytes());
        if (len < ReadableBytes())
            read_index_ += len;
        else if (len == ReadableBytes())
            Reset();
    }
    // Skip [ReadData(), end)
    void SkipUtil(const char* end) {
        assert(ReadData() <= end);
        assert(end <= WriteData());
        Skip(end - ReadData());
    }
    // take out len bytes of readable buffer
    std::string TakeOut(size_t len) {
        assert(len <= ReadableBytes());
        std::string res(ReadData(), len);
        Skip(len);
        return res;
    }
    std::string TakeOutAll() { return TakeOut(ReadableBytes()); }

    void EnsureWritableBytes(size_t len) {
        if (WritableBytes() < len) Extend(len);
        assert(WritableBytes() >= len);
    }
    void Write(const void* data, size_t len) {
        EnsureWritableBytes(len);
        memcpy(WriteData(), data, len);
        write_index_ += len;
        assert(write_index_ <= buffer_.size());
    }
    void Append(std::string_view data) { Write(data.data(), data.size()); }
    void Append(const char* data, size_t len) { Write(data, len); }

    void Prepend(const void* data, size_t len) {
        assert(len <= PrePendedBytes());
        read_index_ -= len;
        memcpy(begin() + read_index_, data, len);
    }

    // read and put data in buffer.
    ssize_t ReadFd(int fd, int* saved_errno);

    const char* FindCRLF() const {
        void* crlf = memmem(ReadData(), ReadableBytes(), kCRLF, 2);
        return crlf == NULL ? nullptr : static_cast<const char*>(crlf);
    }
    const char* FindCRLF(const char* start) const {
        assert(ReadData() <= start);
        assert(start <= WriteData());
        void* crlf = memmem(start, WriteData() - start, kCRLF, 2);
        return crlf == NULL ? nullptr : reinterpret_cast<const char*>(crlf);
    }
    const char* FindEOL() const
    {
        const void* eol = memchr(ReadData(), '\n', ReadableBytes());
        return static_cast<const char*>(eol);
    }
    const char* FindEOL(const char* start) const
    {
        assert(ReadData() <= start);
        assert(start <= WriteData());
        const void* eol = memchr(start, '\n', WriteData() - start);
        return static_cast<const char*>(eol);
    }

   private:
    char* begin() { return buffer_.data(); }
    const char* begin() const { return buffer_.data(); }

    // extend when space isn't enough
    void Extend(size_t len) {
        if (read_index_ - kPrependSize + WritableBytes() < len) {
            buffer_.resize(write_index_ + len);
        }
        if (kPrependSize == read_index_) return;
        auto readable = ReadableBytes();
        // can't use memcpy because of overlap
        memmove(begin() + kPrependSize, ReadData(), readable);
        read_index_ = kPrependSize;
        write_index_ = readable + read_index_;
        assert(readable = ReadableBytes());
    }

    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;

};

#endif  // MYWEBSERVER_BUFFER_H
