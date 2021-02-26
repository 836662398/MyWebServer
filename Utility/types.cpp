//
// Created by rui836662398 on 2021/2/26.
//
#include "types.h"

#include <sstream>

long long IdToInt(std::thread::id id){
    std::stringstream ss;
    ss << id;
    return std::stoll(ss.str());
}