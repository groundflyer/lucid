// -*- C++ -*-
// lucid.cpp

#include <scene/geometry_object.hpp>
#include <cameras/perspective.hpp>
#include <io/image.hpp>

#include <string>

using namespace yapt;

int main(int argc, char *argv[])
{
    size_t resx = 640;
    size_t resy = 480;
    if(argc == 3)
    {
        resx = std::stoul(argv[1]);
        resy = std::stoul(argv[2]);
    }
    const Vec2u res(resx, resy);
    std::cout << "Res: " << resx << ' ' << resy << std::endl;

    const Point sphere_pos(0,1,0);
    const Point disk_pos(-1,1,1);
    const Sphere sphere(sphere_pos, 1);
    const Plane plane(Point(0, 0, 0), Normal(0,1,0));
    std::vector<GenericPrimitive> prims;
    prims.push_back(sphere);
    prims.push_back(plane);
    prims.push_back(Disk(disk_pos, Normal(sphere_pos - disk_pos), 1_r));
    prims.push_back(AABB(Point(-6,0,1), Point(-3,4,3)));

    const Point lp(-2, 5, -3);

    PerspectiveCamera cam(math::radians(120_r),
                          look_at(Point(0,4,-6), sphere_pos));
    Image<float, 3> img(res);

    for(auto it = img.begin(); it != img.end(); ++it)
    {
        const auto ccd = to_camera_coords(it.pos(), res);
        const auto ray = cam(ccd);
        const auto isect = traverse(ray, prims);
        const auto p = get_intersection_pos(ray, isect.first);
        const auto i = -ray.dir;
        const auto n = compute_normal(ray, isect.first, *isect.second);
        const auto L = lp - p;
        const auto Ld = length(L);
        const Normal l(L / Ld);
        const auto shadow = occlusion(Ray(p, l), prims, Range<real>(std::numeric_limits<real>::min() + std::numeric_limits<real>::epsilon() * 1000, Ld));
        const RGB c(std::max(n.dot(l), 0_r) / (math::pow<3>(Ld)) * 30 * !shadow);
        *it = c;
        // *it = fit(n, min(n), max(n));
    }

    write_ppm(img, "render.ppm");

    return 0;
}
