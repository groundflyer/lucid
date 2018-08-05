// -*- C++ -*-
// image.cpp
#include <iostream>

#include <image/image.hpp>
#include <io/image.hpp>

using namespace std;
using namespace yapt;

int main()
{
    cout << boolalpha;

    Image<float, 3> rgb_img(Vec2u(640, 480));

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
        *it = RGB(color);
    }

    write_ppm(rgb_img, "image.ppm");
    return 0;
}
