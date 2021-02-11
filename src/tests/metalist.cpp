// -*- C++ -*-
// repeat.cpp
#include <utility>
#include <utils/intlist.hpp>
#include <utils/typelist.hpp>

using namespace lucid;

static_assert(std::is_same_v<typelist<int, int, int, int>, typename repeat_type<4, int>::type>);

static_assert(std::is_same_v<
              std::integer_sequence<int, -10, 15, 20, -900>,
              join_t<std::integer_sequence<int, -10, 15>, std::integer_sequence<int, 20, -900>>>);

static_assert(
    std::is_same_v<std::integer_sequence<int, 10, 10, 10, 10, 10>, repeat_integers_t<5, int, 10>>);

static_assert(std::is_same_v<typename typelist<bool, int, float, double>::template at<2>, float>);

static_assert(typelist<bool, int, float, double>::template index<float>() == 2);

using rev_seq1 = std::integer_sequence<int, 1, 2, 3, 4>;
using rev_seq2 = std::integer_sequence<int, 4, 3, 2, 1>;
static_assert(std::is_same_v<reverse_t<rev_seq1>, rev_seq2>);

using fill_seq1 = std::integer_sequence<int, 1, 2, 3, 4>;
using fill_seq2 = std::integer_sequence<int, 1, 2, 3, 4, 0, 0, 0>;
static_assert(std::is_same_v<fill_val_t<fill_seq1, 3, 0>, fill_seq2>);

using del_seq1 = std::integer_sequence<int, 1, 2, 3, 4>;
using del_seq2 = std::integer_sequence<int, 1, 2, 4>;
static_assert(std::is_same_v<del_t<del_seq1, 3>, del_seq2>);

int
main()
{
    return 0;
}
