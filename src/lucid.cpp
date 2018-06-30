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
    const Plane plane(Point(0, -2, 0), Normal(0,1,0));
    std::vector<GenericPrimitive> prims;
    prims.push_back(sphere);
    prims.push_back(plane);
    prims.push_back(Disk(disk_pos, Normal(sphere_pos - disk_pos), 1_r));

    PerspectiveCamera cam(math::radians(120_r),
                          look_at(Point(0,1,-3), sphere_pos));
    Image<float, 3> img(res);

    for(auto it = img.begin(); it != img.end(); ++it)
    {
        const auto ccd = to_camera_coords(it.pos(), res);
        const auto ray = cam(ccd);
        const auto isect = traverse(ray, prims);
        const RGB color = isect ? RGB(isect.distance()) * 0.05 : RGB(0);
        *it = yapt::min(color, RGB(1));
    }

    write_ppm(img, "render.ppm");

    return 0;
}
