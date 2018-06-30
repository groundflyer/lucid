// -*- C++ -*-
// perspective.hpp -- 

#include <core/ray.hpp>


namespace yapt
{
    class PerspectiveCamera
    {
        real m_tan_fov_half{1_r};
        Mat4 m_transform{Mat4::unit()};

    public:
        constexpr
        PerspectiveCamera() {}

        template <template <typename, size_t> typename Container>
        constexpr
        PerspectiveCamera(const real& fov,
                          const Mat4_<Container>& transform) :
            m_tan_fov_half(math::tan(fov * 0.5_r)),
            m_transform(transform)
        {}

        template <template <typename, size_t> typename Container>
        constexpr auto
        operator()(const Vec2_<Container>& coords) const noexcept
        {
            const auto& [x, y] = coords;
            const Normal d(x * m_tan_fov_half, y * m_tan_fov_half, 1);
            return apply_transform(m_transform, Ray(Point(0), d));
        }
    };

    constexpr auto
    to_camera_coords(const Vec2& pos, const Vec2& res) noexcept
    { return (pos - res * 0.5_r) / res[0]; }
}
