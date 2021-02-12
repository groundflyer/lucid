// -*- C++ -*-
// repeat.cpp
#include <utility>
#include <utils/intlist.hpp>
#include <utils/typelist.hpp>

using namespace lucid;
using namespace intlist;

static_assert(std::is_same_v<typelist<int, int, int, int>, typename repeat_type<4, int>::type>);

static_assert(std::is_same_v<
              std::integer_sequence<int, -10, 15, 20, -900>,
              join_t<std::integer_sequence<int, -10, 15>, std::integer_sequence<int, 20, -900>>>);

static_assert(std::is_same_v<std::integer_sequence<int, 10, 10, 10, 10, 10>, repeat_t<5, int, 10>>);

static_assert(std::is_same_v<typename typelist<bool, int, float, double>::template at<2>, float>);

static_assert(typelist<bool, int, float, double>::template index<float>() == 2);

using rev_seq1 = std::integer_sequence<int, 1, 2, 3, 4>;
using rev_seq2 = std::integer_sequence<int, 4, 3, 2, 1>;
static_assert(std::is_same_v<reverse_t<rev_seq1>, rev_seq2>);

using fill_seq1 = std::integer_sequence<int, 1, 2, 3, 4>;
using fill_seq2 = std::integer_sequence<int, 1, 2, 3, 4, 0, 0, 0>;
static_assert(std::is_same_v<nappend_t<fill_seq1, 3, 0>, fill_seq2>);

using del_seq1 = std::integer_sequence<unsigned, 1, 2, 3, 4>;
using del_seq2 = std::integer_sequence<unsigned, 1, 2, 4>;
static_assert(std::is_same_v<del_t<del_seq1, 3u>, del_seq2>);

using lcnt = lehmer_code_t<463>;
using lcnv = std::index_sequence<3, 4, 1, 0, 1, 0>;
static_assert(std::is_same_v<lcnv, lcnt>);

constexpr std::size_t L = 3;
using pm_seq0           = std::make_index_sequence<L>;
using pm_seq1           = std::index_sequence<0, 2, 1>;
using pm_seq2           = std::index_sequence<1, 0, 2>;
using pm_seq3           = std::index_sequence<1, 2, 0>;
using pm_seq4           = std::index_sequence<2, 0, 1>;
using pm_seq5           = std::index_sequence<2, 1, 0>;
static_assert(std::is_same_v<nlperm_t<0, L>, pm_seq0>);
static_assert(std::is_same_v<nlperm_t<1, L>, pm_seq1>);
static_assert(std::is_same_v<nlperm_t<2, L>, pm_seq2>);
static_assert(std::is_same_v<nlperm_t<3, L>, pm_seq3>);
static_assert(std::is_same_v<nlperm_t<4, L>, pm_seq4>);
static_assert(std::is_same_v<nlperm_t<5, L>, pm_seq5>);

int
main()
{
    return 0;
}
