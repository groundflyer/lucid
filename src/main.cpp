// -*- C++ -*-
// yapt.cpp --
#include <limits>

#include <io/image.hpp>
#include <cameras/perspective.hpp>
#include <core/geo/sphere.hpp>
#include <core/geo/triangle.hpp>
#include <core/geo/plane.hpp>
#include <core/geo/disk.hpp>
#include <core/geo/bbox.hpp>


using namespace std;
using namespace yapt;


int main()
{
    Vec2u res (640,480);
    Film film(4, res);
    Sphere sphere(Vec3(0,0,2), 0.5);
    TriangleMesh mesh(3, 1, 0, 0);
    mesh.add_vertex(Vec3(-0.5, -0.5, 2.));
    mesh.add_vertex(Vec3(-0.5, 0.5, 2.));
    mesh.add_vertex(Vec3(0.5, -0.5, 2.));
    mesh.add_triangle(0,1,2);
    Triangle tri = Triangle(&mesh, 0);
    // Plane plane(Vec3(0., -0.5, 0), Vec3(0,1,0));
    // Disk disk(Vec3(0,0,2), Vec3(0,0,1), 0.75);
    AABB box(Vec3(-.25,-.25,1.5), Vec3(.25,.25,2.));

    real fovx = radians(real(90));
    real ratio = real(res.x()) / real(res.y());
    real fovy = fovx - std::fmod(fovx * ratio, 1) * 2;
    PerspectiveCamera cam(fovx, fovy);

    real t_min = 0;
    real t_max = std::numeric_limits<real>::infinity();

    for (size_t i = 0; i < res.x() * res.y(); ++i)
	{
	    unsigned x = i % res.x();
	    unsigned y = (i - x) / res.x();
	    Vec2u pos(x, y);
	    Vec2 ndc = Vec2(pos) / Vec2(res);
	    Ray ray = cam.generate_ray(ndc);
	    // auto isect = sphere.intersect(ray, t_min, t_max);
	    auto isect = tri.intersect(ray, t_min, t_max);
	    // auto isect = plane.intersect(ray, t_min, t_max);
	    // auto isect = disk.intersect(ray, t_min, t_max);
	    // auto isect = box.intersect(ray, t_min, t_max);
	    RGBA clr;
	    if (isect)
		clr = RGBA(0, 0.8, 0.5, 1);

	    film.contribute(pos, clr.begin());
	}

    int status = write_image(film, "out.png");

    return status;
}










