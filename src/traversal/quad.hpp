// -*- C++ -*-
// quad.hpp
//

#pragma once

#include "triangle.hpp"
#include <base/range.hpp>
#include <base/rng.hpp>


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Quad_
    {
        Point_<Container> v00;
        Point_<Container> v01;
        Point_<Container> v11;
        Point_<Container> v10;

        constexpr
        Quad_() {}

		template <template <typename, size_t> typename Container1,
                  template <typename, size_t> typename Container2,
                  template <typename, size_t> typename Container3,
                  template <typename, size_t> typename Container4>
        constexpr
        Quad_(const Point_<Container1>& _v00,
              const Point_<Container2>& _v01,
              const Point_<Container3>& _v11,
              const Point_<Container4>& _v10) :
            v00(_v00), v01(_v01), v11(_v11), v10(_v10)
        {}
    };

    template <template <typename, size_t> typename Container>
    Quad_(const Point_<Container>&,
          const Point_<Container>&,
          const Point_<Container>&,
          const Point_<Container>&) -> Quad_<Container>;

    using Quad = Quad_<std::array>;

    template <template <typename, size_t> typename QuadContainer,
			  template <typename, size_t> typename RayContainer>
    constexpr auto
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

        const constexpr Range range01(0_r, 1_r);

        if(!range01.encloses(alpha))
            return Intersection();

        const auto Q = T.cross(e01);
        const auto beta = d.dot(Q) / det;
        if(!range01.encloses(beta))
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

            if(!range01.encloses(u))
                u = C / QQ;

            v = beta / (u * (b11 - 1) + 1);
        }

        return Intersection{true, t, Vec2(u, v)};
    }

	template <template <typename, size_t> typename QuadContainer,
			  template <typename, size_t> typename RayContainer,
              template <typename, size_t> typename IsectContainer>
    constexpr auto
    normal(const Ray_<RayContainer>&,
           const Intersection_<IsectContainer>&,
           const Quad_<QuadContainer>& prim) noexcept
    {
        const auto e01 = prim.v10 - prim.v00;
        const auto e03 = prim.v01 - prim.v00;
        return Normal(e01.cross(e03));
    }

	template <template <typename, size_t> typename SContainer,
              template <typename, size_t> typename PContainer>
    constexpr auto
    sample(const Vec2_<SContainer>& s,
           const Quad_<PContainer>& prim) noexcept
    {
        const auto& a = prim.v00;
        const auto& b = prim.v11;
        const auto& [t1, t2] = s;
        const auto& c = t1 > 0.5_r ? prim.v01 : prim.v10;
        return Point(impl::triangle_sample(Vec2(resample(t1), t2), a, b, c));
    }

    template <template <typename, size_t> typename Container>
    constexpr auto
    centroid(const Quad_<Container>& prim) noexcept
    {
        const auto& [v0, v1, v2, v3] = prim;
        return Point((v0 + v1 + v2 + v3) * 0.25_r);
    }
}
