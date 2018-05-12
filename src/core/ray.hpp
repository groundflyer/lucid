// -*- C++ -*-
// ray.hpp --
//

#pragma once

#include "types.hpp"
#include "transforms.hpp"


namespace yapt
{
    template <template <typename, size_t> typename Container>
    struct Ray_
    {
		Point_<Container> origin;
		Normal_<Container> dir;

		constexpr
		Ray_() {};

		template <template <typename, size_t> typename Container1,
				  template <typename, size_t> typename Container2>
		constexpr
		Ray_(const Point_<Container1>& _origin,
			 const Normal_<Container2>& _dir) : origin(_origin), dir(_dir) {}
    };

	template <template <typename, size_t> typename Container>
	Ray_(const Point_<Container>&, const Normal_<Container>&) -> Ray_<Container>;

	using Ray = Ray_<std::array>;


    template <template <typename, size_t> typename MatContainer,
			  template <typename, size_t> typename RayContainer>
    constexpr auto
    apply_transform(const Mat4_<MatContainer>& t,
					const Ray_<RayContainer>& ray)
    { return Ray_<RayContainer>(apply_transform(t, ray.origin), apply_transform(t, ray.dir)); }
}
