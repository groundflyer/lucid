// -*- C++ -*-
// image.cpp
#include <iostream>

#include <image/image.hpp>
#include <image/io.hpp>

using namespace std;
using namespace lucid;

int main()
{
    cout << boolalpha;

    Image<unsigned char, 3> rgb_img(Vec2u(640, 480));

    cout << "Res: " << rgb_img.res() << endl;
    cout << "Num pixels: " << rgb_img.num_pixels() << endl;
    cout << "Size: " << rgb_img.size() << endl;

    for (auto it = rgb_img.begin(); it != rgb_img.end(); ++it)
    {
        const auto vv = Vec2(it.pos()) / Vec2(rgb_img.res());
        if(any(vv > 1_r))
            cout << vv << ':' << it.pos() << endl;
        const Vec2 ndc(vv);
        const RGB color(ndc);
        *it = Vector<unsigned char, 3, std::array>(RGB(color) * 255);
    }

    write_ppm(rgb_img, "image.ppm");
    return 0;
}
