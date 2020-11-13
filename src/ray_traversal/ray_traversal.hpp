// -*- C++ -*-
// ray_traversal.hpp
//

#pragma once

#include <base/transforms.hpp>
#include <base/types.hpp>

namespace lucid
{
template <template <typename, std::size_t> typename Container>
struct Ray_
{
    Vec3_<Container> origin;
    Vec3_<Container> dir;

    constexpr Ray_() noexcept {};

    template <template <typename, std::size_t> typename Container1,
              template <typename, std::size_t>
              typename Container2>
    constexpr Ray_(const Vec3_<Container1>& _origin, const Vec3_<Container2>& _dir) noexcept :
        origin(_origin), dir(normalize(_dir))
    {
    }
};

template <template <typename, std::size_t> typename Container>
Ray_(const Vec3_<Container>&, const Vec3_<Container>&) -> Ray_<Container>;

using Ray = Ray_<std::array>;

template <template <typename, std::size_t> typename Container>
struct Intersection_
{
    bool             intersect;
    real             t;
    Vec2_<Container> st;

    constexpr Intersection_() noexcept :
        intersect(false), t(std::numeric_limits<real>::max()), st(Vec2(0))
    {
    }

    template <template <typename, std::size_t> typename Container2>
    constexpr Intersection_(const bool               _intersect,
                            const real&              _t,
                            const Vec2_<Container2>& _st) noexcept :
        intersect(_intersect),
        t(intersect ? _t : std::numeric_limits<real>::max()), st(_st)
    {
    }

    constexpr operator bool() const noexcept { return intersect; }
};

template <template <typename, std::size_t> typename Container>
Intersection_(const bool, const real&, const Vec2_<Container>&)->Intersection_<Container>;

using Intersection = Intersection_<std::array>;


namespace fn
{
template <template <typename, std::size_t> typename MatContainer,
          template <typename, std::size_t>
          typename RayContainer>
constexpr auto
apply_transform(const Mat4_<MatContainer>& t, const Ray_<RayContainer>& ray) noexcept
{
    return Ray(apply_transform_p(t, ray.origin), apply_transform_n(t, ray.dir));
}
} // namespace fn
} // namespace lucid
