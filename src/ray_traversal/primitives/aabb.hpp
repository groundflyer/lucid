// -*- C++ -*-
// bbox.hpp --
//

#pragma once

#include <ray_traversal/ray_traversal.hpp>

#include <base/rng.hpp>

#include <utils/identity.hpp>

namespace lucid
{
// axis-aligned bounding box
template <template <typename, size_t> typename Container>
struct AABB_
{
    Vec3_<Container> vmin;
    Vec3_<Container> vmax;

    constexpr AABB_() noexcept {};

    template <template <typename, size_t> typename Container1,
              template <typename, size_t>
              typename Container2>
    constexpr AABB_(const Vec3_<Container1>& _vmin, const Vec3_<Container2>& _vmax) noexcept :
        vmin(lucid::min(_vmin, _vmax)), vmax(lucid::max(_vmin, _vmax))
    {
    }

    constexpr const auto& operator[](const bool i) const noexcept { return i ? vmax : vmin; }

    constexpr auto& operator[](const bool i) noexcept { return i ? vmax : vmin; }

    template <typename Idxs>
    constexpr auto
    operator[](const Idxs& idxs) const noexcept
    {
        Vec3 ret{};
        for(size_t i = 0; i < 3; ++i) ret[i] = (*this)[bool{idxs[i]}][i];
        return ret;
    }
};

template <template <typename, size_t> typename Container>
AABB_(const Vec3_<Container>&, const Vec3_<Container>&) -> AABB_<Container>;

using AABB = AABB_<std::array>;

template <template <typename, size_t> typename AABBContainer,
          template <typename, size_t>
          typename RayContainer>
constexpr Intersection
intersect(const Ray_<RayContainer>& ray, const AABB_<AABBContainer>& prim) noexcept
{
    const auto& [o, d]             = ray;
    const Vec3 inv_d               = Vec3{1_r} / d;
    const auto sign                = inv_d < 0_r;
    const Vec3 vmin                = inv_d * (prim[sign] - o);
    const Vec3 vmax                = inv_d * (prim[!sign] - o);
    const auto& [xmin, ymin, zmin] = vmin;
    const auto& [xmax, ymax, zmax] = vmax;
    const real& tmin1              = std::max(ymin, xmin);
    const real& tmax1              = std::min(ymax, xmax);
    const real& tmin2              = std::max(zmin, tmin1);
    const bool  intersected        = xmin <= ymax && ymin <= xmax && tmin1 <= zmax && zmin <= tmax1;
    return Intersection{intersected && !std::signbit(tmin2), tmin2, Vec2{}};
}

template <template <typename, size_t> typename AABBContainer,
          template <typename, size_t>
          typename RayContainer,
          template <typename, size_t>
          typename IsectContainer>
constexpr Vec3
normal(const Ray_<RayContainer>&            ray,
       const Intersection_<IsectContainer>& isect,
       const AABB_<AABBContainer>&          prim) noexcept
{
    const auto& [o, d] = ray;
    const Vec3 pos     = o + d * isect.t;
    const real nsign[2]{-1_r, 1_r};
    Vec3       vd[2]{};
    real       md[2]{};
    for(unsigned i = 0; i < 2; ++i)
    {
        const Vec3 ad = lucid::abs(pos - prim[bool(i)]);
        md[i]         = lucid::min(ad);
        vd[i]         = Vec3(ad == md[i]);
    }
    const auto pp = md[0] > md[1];
    return normalize(vd[pp] * nsign[pp]);
}

namespace detail
{
template <template <typename, size_t> typename Container>
constexpr auto
diag(const AABB_<Container>& prim, const unsigned shift, const bool side) noexcept
{
    const auto& [vmin, vmax] = prim;
    const Vec3& startpoint   = side ? vmin : vmax;
    const Vec3  diag         = side ? vmax - vmin : vmin - vmax;
    return std::pair(startpoint, startpoint + diag * roll(Vec3(1_r, 1_r, 0_r), shift));
}
} // namespace detail

template <template <typename, size_t> typename SContainer,
          template <typename, size_t>
          typename PContainer>
constexpr Vec3
sample(const Vec2_<SContainer>& s, const AABB_<PContainer>& prim) noexcept
{
    const auto& [s1, s2] = s;
    const real     s3    = math::fmod(2_r * (s1 + s2), 1_r);
    const unsigned shift = static_cast<unsigned>(300_r * math::fmod(s1 + s2 + s3, 1_r));
    const auto [a, b]    = detail::diag(prim, shift, s3 > 0.5_r);
    return lerp(a, b, roll(Vec3(resample(s1), resample(s2), 0_r), shift));
}

template <template <typename, size_t> typename Container>
constexpr Vec3
centroid(const AABB_<Container>& prim) noexcept
{
    const auto& [vmin, vmax] = prim;
    return (vmin + vmax) * 0.5_r;
}

template <template <typename, size_t> typename Container>
constexpr AABB
bound(const AABB_<Container>& prim) noexcept
{
    return prim;
}

template <template <typename, size_t> typename MatContainer,
          template <typename, size_t>
          typename PrimContainer>
constexpr AABB
apply_transform(const Mat4_<MatContainer>& t, const AABB_<PrimContainer>& prim) noexcept
{
    const auto& [vmin, vmax] = prim;
    return AABB(apply_transform_p(t, vmin), apply_transform_p(t, vmax));
}
} // namespace lucid
