// -*- C++ -*-
// image.cpp --

#include "image.h"
#include <OpenImageIO/imageio.h>


namespace yapt
{
    bool
    write_image(const Film & film,
		const std::string & filename) noexcept
    {
	OIIO_NAMESPACE_USING;
	
	ImageOutput *out = ImageOutput::create(filename);

	if (!out)
	    return false;

	ImageSpec spec(film.res().x(), film.res().y(),
		       film.num_channels(),
		       TypeDesc::FLOAT);

	out->open(filename, spec);
	bool status = out->write_image(TypeDesc::FLOAT, film.cbegin());
	out->close();
	delete out;

	return status;
    }
}
