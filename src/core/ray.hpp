// -*- C++ -*-
// ray.hpp --
//

#pragma once

#include "types.hpp"


namespace yapt
{
    template <template <typename, size_t> typename PointContainer,
			  template <typename, size_t> typename NormalContainer>
    struct Ray_
    {
		const Point_<PointContainer> origin;
		const Normal_<NormalContainer> dir;

		constexpr
		Ray_() = delete;

		template <template <typename, size_t> typename Container1,
				  template <typename, size_t> typename Container2>
		constexpr
		Ray_(const Point_<Container1>& _origin,
			 const Normal_<Container2>& _dir) : origin(_origin), dir(_dir) {}
    };

	template <template <typename, size_t> typename Container>
	Ray_(const Point_<Container>&, const Normal_<Container>&) -> Ray_<Container, Container>;

	using Ray = Ray_<std::array, std::array>;


    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename RayPContainer,
			  template <typename, size_t> typename RayNContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer>& t,
					const Ray_<RayPContainer, RayNContainer>& ray)
    { return Ray_<RayPContainer, RayNContainer>(apply_transform(t, ray.origin), apply_transform(t, ray.dir)); }
}
