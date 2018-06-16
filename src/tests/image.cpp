// -*- C++ -*-
// image.cpp
#include <iostream>

#include <image/image.hpp>
#include <io/image.hpp>

using namespace std;
using namespace yapt;

int main()
{
    Image<float, 3> rgb_img(Res(640, 480));

    cout << "Res: " << rgb_img.res() << endl;
    cout << "Num pixels: " << rgb_img.num_pixels() << endl;
    cout << "Size: " << rgb_img.size() << endl;

    for (auto elem : rgb_img)
        elem = RGB(0, 1, 1);

    cout << rgb_img[479][639] << endl;
    cout << boolalpha;
    cout << (&(rgb_img[100][100][0]) == &(rgb_img.at(100, 100)[0])) << endl;

    write_ppm(rgb_img, "image.ppm");
    return 0;
}
