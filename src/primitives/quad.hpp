// -*- C++ -*-
// quad.hpp
//

#pragma once

#include "triangle.hpp"
#include <utils/range.hpp>
#include <base/rng.hpp>


namespace yapt
{
    // v00, v01, v11, v10
    template <template <typename, size_t> typename Container>
    using Quad_ = std::array<Point_<Container>, 4>;

    using Quad = Quad_<std::array>;

    template <template <typename, size_t> typename QuadContainer,
			  template <typename, size_t> typename RayContainer>
    constexpr Intersection
    intersect(const Ray_<RayContainer>& ray,
              const Quad_<QuadContainer>& prim) noexcept
    {
        const auto& [o, d] = ray;
        const auto& [v00, v01, v11, v10] = prim;
        const auto e01 = v10 - v00;
        const auto e03 = v01 - v00;
        const auto p = d.cross(e03);
        const auto det = e01.dot(p);

        if(math::abs(det) < std::numeric_limits<real>::min())
            return Intersection();

        const auto T = o - v00;
        const auto alpha = T.dot(p) / det;

        const constexpr auto range01 = range(0_r, 1_r);

        if(!range01(alpha))
            return Intersection();

        const auto Q = T.cross(e01);
        const auto beta = d.dot(Q) / det;
        if(!range01(beta))
            return Intersection();

        if((alpha + beta) > 1)
        {
            const auto e23 = v01 - v11;
            const auto e21 = v10 - v11;
            const auto p_ = d.cross(e21);
            const auto det_ = e23.dot(p_);

            if(math::abs(det_) < std::numeric_limits<real>::min())
                return Intersection();

            const auto T_ = o - v11;
            const auto alpha_ = T_.dot(p_) / det_;

            if(alpha_ < 0)
                return Intersection();

            const auto Q_ = T_.cross(e23);
            const auto beta_ = d.dot(Q_) / det_;
            if(beta_ < 0)
                return Intersection();
        }

        const auto t = e03.dot(Q) / det;

        if(t < 0)
            return Intersection();

        const auto e02 = v11 - v00;
        const auto N = e01.cross(e03);
        const auto aN = abs(N);
        const auto& [Nx, Ny, Nz] = N;
        const auto& [aNx, aNy, aNz] = aN;

        real a11{}, b11{};
        if((aNx >= aNy) && (aNx >= aNz))
        {
            a11 = (e02[1] * e03[2] - e02[2] * e03[1]) / Nx;
            b11 = (e01[1] * e02[2] - e01[2] * e02[1]) / Nx;
        }
        else if((aNy >= aNx) && (aNy >= aNz))
        {
            a11 = (e02[2] * e03[0] - e02[0] * e03[2]) / Ny;
            b11 = (e01[2] * e02[0] - e01[0] * e02[2]) / Ny;
        }
        else
        {
            a11 = (e02[0] * e03[1] - e02[1] * e03[0]) / Nz;
            b11 = (e01[0] * e02[1] - e01[1] * e02[0]) / Nz;
        }

        real u{}, v{};
        if(math::abs(a11 - 1) < std::numeric_limits<real>::min())
        {
            u = alpha;
            if(math::abs(b11 - 1) < std::numeric_limits<real>::min())
                v = beta;
            else
                v = beta / (u * (b11 - 1) + 1);
        }
        else if(math::abs(b11 - 1) < std::numeric_limits<real>::min())
        {
            v = beta;
            u = alpha / (v * (a11 - 1) + 1);
        }
        else
        {
            const auto A = -(b11 - 1);
            const auto B = alpha * (b11 - 1) - beta * (a11 - 1) - 1;
            const auto C = alpha;
            const auto Delta = B*B - 4 * A * C;
            const auto QQ = -0.5_r * (B + std::copysign(math::sqrt(Delta), B));
            u = QQ / A;

            if(!range01(u))
                u = C / QQ;

            v = beta / (u * (b11 - 1) + 1);
        }

        return Intersection{true, t, Vec2(u, v)};
    }

	template <template <typename, size_t> typename QuadContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr Normal
    normal(const Ray_<RayContainer>&,
           const Intersection_<IsectContainer>&,
           const Quad_<QuadContainer>& prim) noexcept
    {
        const auto e01 = std::get<3>(prim) - std::get<0>(prim);
        const auto e03 = std::get<1>(prim) - std::get<0>(prim);
        return Normal(e01.cross(e03));
    }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr Point
    sample(const Vec2_<SContainer>& s,
           const Quad_<PContainer>& prim) noexcept
    {
        const auto& [a, c1, b, c2] = prim;
        const auto& [t1, t2] = s;
        const auto& c = t1 > 0.5_r ? c1 : c2;
        return Point(detail::triangle_sample(Vec2(resample(t1), t2), a, b, c));
    }

    template <template <typename, size_t> typename Container>
    constexpr Point
    centroid(const Quad_<Container>& prim) noexcept
    { return centroid(detail::bound(prim)); }

    template <template <typename, size_t> typename Container>
    constexpr AABB
    bound(const Quad_<Container>& prim) noexcept
    { return detail::bound(prim); }
}
