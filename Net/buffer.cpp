//
// Created by rui836662398 on 2021/2/27.
//

#include "buffer.h"

#include <sys/uio.h>

const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kDefaultSize;
const size_t Buffer::kPrependSize;

ssize_t Buffer::ReadFd(int fd, int *saved_errno) {
    // stack space acts as spare
    char extrabuf[65536];
    struct iovec vec[2];
    size_t writable = WritableBytes();
    vec[0].iov_base = WriteData();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // need't extrabuf if writable is big enough
    int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0) {
        *saved_errno = errno;
    } else if (n < writable) {
        write_index_ += n;
    } else {
        write_index_ = buffer_.size();
        Append(extrabuf, n - writable);
    }
    return n;
}
