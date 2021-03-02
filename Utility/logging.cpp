//
// Created by rui836662398 on 2021/3/1.
//
#include "logging.h"

thread_local char t_errnobuf[512];

const char* strerror_tl(int saved_errno) {
    return strerror_r(saved_errno, t_errnobuf, sizeof t_errnobuf);
}
