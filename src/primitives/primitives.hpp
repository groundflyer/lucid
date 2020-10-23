// -*- C++ -*-
// primitives.hpp
//

#pragma once

#include "aabb.hpp"
#include "disk.hpp"
#include "generic.hpp"
#include "quad.hpp"
#include "sphere.hpp"
#include "triangle.hpp"

namespace lucid
{
namespace prim_fn
{
struct intersect_fn
{
    template <typename... Args>
    constexpr Intersection
    operator()(const Args&... args) const noexcept
    {
        return prim_fn::intersect(args...);
    }
};

struct normal_fn
{
    template <typename... Args>
    constexpr Vec3
    operator()(const Args&... args) const noexcept
    {
        return prim_fn::normal(args...);
    }
};

struct sample_fn
{
    template <typename... Args>
    constexpr Vec3
    operator()(const Args&... args) const noexcept
    {
        return prim_fn::sample(args...);
    }
};

struct bound_fn
{
    template <typename... Args>
    constexpr AABB
    operator()(const Args&... args) const noexcept
    {
        return prim_fn::bound(args...);
    }
};

struct apply_transform_fn
{
    template <typename... Args>
    constexpr auto
    operator()(const Args&... args) const noexcept
    {
        return prim_fn::apply_transform(args...);
    }
};
} // namespace prim_fn

static constexpr prim_fn::intersect_fn intersect;
static constexpr prim_fn::normal_fn    normal;
static constexpr prim_fn::sample_fn    sample;
static constexpr prim_fn::bound_fn     bound;
// static constexpr prim_fn::apply_transform_fn apply_transform;
} // namespace lucid
