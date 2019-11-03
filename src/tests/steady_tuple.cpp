// -*- C++ -*-
// steady_tuple.cpp
#include <utils/steady_tuple.hpp>

struct A
{
    int a;
    float b;
    A(int _a) : a(_a), b(10.5f) {}

    A() = delete;
    A(const A&) = delete;
    A(A&&) = delete;
    A& operator=(const A&) = delete;
    A& operator=(A&&) = delete;
};

struct B
{
    int c;
    float d;
    B(int _c) : c(_c), d(-4.2f) {}

    B() = delete;
    B(const B&) = delete;
    B(B&&) = delete;
    B& operator=(const B&) = delete;
    B& operator=(B&&) = delete;
};


int main()
{
    lucid::steady_tuple<A, B> tpl(lucid::uniform_init{}, 100500);
    return tpl.template get<A>().a != tpl.template get<B>().c;
}
