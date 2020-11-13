// -*- C++ -*-
// ray_tr.hpp
// apply_transform declaration for transforms.hpp

#pragma once

#include <base/types.hpp>

namespace lucid
{
template <template <typename, std::size_t> typename Container>
struct Ray_;

using Ray = Ray_<std::array>;

namespace fn
{
template <template <typename, std::size_t> typename MatContainer,
          template <typename, std::size_t>
          typename RayContainer>
constexpr auto
apply_transform(const Mat4_<MatContainer>&, const Ray_<RayContainer>&) noexcept;
}
} // namespace lucid
