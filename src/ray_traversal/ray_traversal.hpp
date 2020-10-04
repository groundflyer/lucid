// -*- C++ -*-
// ray_traversal.hpp
//

#pragma once

#include <base/transforms.hpp>
#include <base/types.hpp>

namespace lucid
{
template <template <typename, size_t> typename Container>
struct Ray_
{
    Vec3_<Container> origin;
    Vec3_<Container> dir;

    constexpr Ray_() noexcept {};

    template <template <typename, size_t> typename Container1,
              template <typename, size_t>
              typename Container2>
    constexpr Ray_(const Vec3_<Container1>& _origin, const Vec3_<Container2>& _dir) noexcept :
        origin(_origin), dir(normalize(_dir))
    {
    }
};

template <template <typename, size_t> typename Container>
Ray_(const Vec3_<Container>&, const Vec3_<Container>&) -> Ray_<Container>;

using Ray = Ray_<std::array>;

template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr auto
apply_transform(const Mat4_<MatContainer>& t, const Ray_<RayContainer>& ray) noexcept
{
    return Ray(apply_transform(t, ray.origin), apply_transform(t, ray.dir));
}

template <template <typename, size_t> typename Container>
struct Intersection_
{
    bool             intersect;
    real             t;
    Vec2_<Container> st;

    constexpr Intersection_() noexcept :
        intersect(false), t(std::numeric_limits<real>::max()), st(Vec2(0))
    {
    }

    template <template <typename, size_t> typename Container2>
    constexpr Intersection_(const bool               _intersect,
                            const real&              _t,
                            const Vec2_<Container2>& _st) noexcept :
        intersect(_intersect),
        t(intersect ? _t : std::numeric_limits<real>::max()), st(_st)
    {
    }

    constexpr operator bool() const noexcept { return intersect; }

    // static constexpr Intersection_ miss;
};

template <template <typename, size_t> typename Container>
Intersection_(const bool, const real&, const Vec2_<Container>&)->Intersection_<Container>;

using Intersection = Intersection_<std::array>;

namespace detail
{
static constexpr Intersection miss{};

template <template <typename, std::size_t> typename Container,
          typename PrimsTuple,
          std::size_t... Ids>
constexpr auto
hider_impl(const Ray_<Container>& ray,
           const PrimsTuple&      prims,
           std::index_sequence<Ids...>) noexcept
{
    return closest(std::tuple{intersect(ray, std::get<Ids>(prims))...});
}
} // namespace detail

template <typename Intersections>
constexpr decltype(auto)
closest(const Intersections& isects) noexcept
{
    return reduce_tuple(
        [](const auto& a, const auto& b) noexcept { return a.second.t < b.second.t ? (a) : (b); },
        std::pair<std::size_t, const Intersection&>{0ul, detail::miss},
        enumerate(isects));
}

template <template <typename, std::size_t> typename Container, typename PrimsTuple>
constexpr auto
hider(const Ray_<Container>& ray, const PrimsTuple& prims) noexcept
{
    return detail::hider_impl(
        ray, prims, std::make_index_sequence<std::tuple_size_v<PrimsTuple>>{});
}
} // namespace lucid
