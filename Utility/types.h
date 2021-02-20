//
// Created by rui836662398 on 2021/2/7.
//

#ifndef MYWEBSERVER_TYPES_H
#define MYWEBSERVER_TYPES_H

// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
template<typename To, typename From>
inline To implicit_cast(From const &f)
{
    return f;
}

template<typename To, typename From>     // use like this: down_cast<T*>(foo);
inline To down_cast(From* f)                     // so we only accept pointers
{

#ifndef NDEBUG
    assert(f == NULL || dynamic_cast<To>(f) != NULL);  // RTTI: debug mode only!
#endif
    return static_cast<To>(f);
}

#endif //MYWEBSERVER_TYPES_H
