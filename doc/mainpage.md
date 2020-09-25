Development documentation {#mainpage}
=====================================

Lucid is an experimental path-tracer where the experiment is rather *how* to implement than *what* to implement.

The goal is to find efficient abstractions that allow to implement complex algorithms without performance sacrifice.

The core of Lucid is a full-typed, lock-free [task dispatcher](@ref lucid::Dispatcher).

This task dispatcher doesn't erase type information as thread pools usually do,
allowing compilers to apply more aggressive optimizations and to group tasks into batches using their types.

# Building

* CMake 3.16 or newer
* LLVM Clang and libc++ version 10 or newer
