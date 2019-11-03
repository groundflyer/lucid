# Work in progress

Lucid is an experimental path-tracer where the experiment is rather *how* to implement than *what* to implement.

The goal is to find efficient abstractions that allow implementing complex algorithms without sacrificing performance
thus we extensively use template metaprogramming.

The core of Lucid is full-typed, lock-free [task dispatcher](src/utils/dispatcher.hpp)
based on [concurrent queue](https://github.com/cameron314/concurrentqueue) and [typelist](src/utils/typelist.hpp)
with active waiting thread pool.

This task dispatcher doesn't erase type information as thread pools usually do,
allowing compilers to apply more aggressive optimizations and also we can group tasks naturally with their types.

# Building

* CMake 3.9 or newer
* Clang 7.1.0 or newer
