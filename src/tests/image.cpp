// -*- C++ -*-
// image.cpp
#include <iostream>

#include <io/image.hpp>

using namespace std;
using namespace yapt;

int main()
{
    Image<4> rgba_img(Res(640, 480));

    cout << "Res: " << rgba_img.res() << endl;
    cout << "Size: " << rgba_img.size() << endl;

    for (auto elem : rgba_img)
        elem = 1;

    cout << rgba_img[100] << endl;

    // const string filename = "white.png";
    // try
    // {
    //     write_image(rgba_img, filename);
    // }
    // catch (const std::runtime_error& ex)
    // {
    //     cerr << ex.what() << endl;
    //     return 1;
    // }
    // cout << filename << " has written\n";
    return 0;
}
