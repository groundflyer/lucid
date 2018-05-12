// -*- C++ -*-
// perspective.hpp -- 

#include <core/ray.hpp>


namespace yapt
{
    class PerspectiveCamera
    {
        real m_tan_fovx_half{1_r};
        real m_tan_fovy_half{1_r};
        Mat4 m_transform{Mat4::unit()};

    public:
        constexpr
        PerspectiveCamera() {}

        template <template <typename, size_t> typename Container>
        constexpr
        PerspectiveCamera(const real& fovx,
                          const real& fovy,
                          const Mat4_<Container>& transform) :
            m_tan_fovx_half(math::tan(fovx * 0.5_r)),
            m_tan_fovy_half(math::tan(fovy * 0.5_r)),
            m_transform(transform)
        {}

        template <template <typename, size_t> typename Container>
        constexpr auto
        operator()(const NDC_<Container>& ndc) const noexcept
        {
            const auto& [x, y] = ndc;

            const Normal d((x - 0.5_r) * m_tan_fovx_half,
                          (0.5_r - y) * m_tan_fovy_half,
                          1);

            return apply_transform(m_transform, Ray(Point(0), d));
        }
    };
}
