// Needed for PNG functions
#define _CRT_SECURE_NO_WARNINGS

#include "../../includes/gtl/GameTextureLoader.hpp"
#include <string>
#include <cstdio>
#include "../../source/external/lpng128/png.h"
#include <boost/shared_ptr.hpp>

void savePNG(std::string filename, GameTextureLoader3::ImagePtr img)
{
	typedef boost::shared_ptr<FILE> fileptr; 

	int colour_type;
	int numChan = 0;
	
	switch(img->getFormat())
	{
	case GameTextureLoader3::FORMAT_DXT1:
	case GameTextureLoader3::FORMAT_DXT2:
	case GameTextureLoader3::FORMAT_DXT3:
	case GameTextureLoader3::FORMAT_DXT4:
	case GameTextureLoader3::FORMAT_DXT5:
	case GameTextureLoader3::FORMAT_3DC:
		img = GameTextureLoader3::Decompress(img);
		break;
	default:
		break;
	}

	switch(img->getFormat())
	{
	case GameTextureLoader3::FORMAT_RGB:
	case GameTextureLoader3::FORMAT_BGR:
		colour_type = PNG_COLOR_TYPE_RGB;
		numChan = 3;
		break;
	case GameTextureLoader3::FORMAT_RGBA:
		colour_type = PNG_COLOR_TYPE_RGB_ALPHA;
		numChan = 4;
		break;
	default:
		return;
	}
	size_t pos = 0; 
	while(( pos = filename.find("/",pos)) != std::string::npos)
	{
		filename.replace(pos,1,"_");
	}

	pos = 0; 
	while(( pos = filename.find(":",pos)) != std::string::npos)
	{
		filename.replace(pos,1,"_");
	}

	const char * pfilename = filename.c_str();
	fileptr file(fopen(pfilename,"wb"),fclose);

	png_voidp user_error_ptr = 0;
	png_error_ptr user_error_fn = 0, user_warning_fn = 0;
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		user_error_ptr, user_error_fn, user_warning_fn);

	if(!png_ptr)
		return;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if(!info_ptr)
	{
		png_destroy_write_struct(&png_ptr,(png_infopp)0);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return;
	}

	png_init_io(png_ptr, file.get());
	png_uint_32 width = img->getWidth();
	png_uint_32 height = img->getHeight();
	
	const int bit_depth = img->getColourDepth() / numChan;	// so 8bit for 24bit 3 channel image
	const int interlace_type = PNG_INTERLACE_NONE;
	const int compression_type = PNG_COMPRESSION_TYPE_DEFAULT;
	const int filter_method = PNG_FILTER_TYPE_DEFAULT;

	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, colour_type, interlace_type,
		 compression_type, filter_method);
	png_write_info(png_ptr, info_ptr);

	const int byte_depth = bit_depth/8;
	  
	const int rowwidth = width * byte_depth * numChan;
	png_bytep *imgData = new png_bytep[height];
	for (int i = 0; i < height; i++)
	{
		imgData[i] = new png_byte[rowwidth];
	}

	for (int row = 0; row < height; row++)
	{
		unsigned char *ptr = imgData[row];
		
		unsigned char *src = img->getDataPtr() +(row * rowwidth);
		unsigned char *srcend = src + rowwidth;

		std::copy(src,srcend,ptr);
	}
	
	png_write_image(png_ptr, imgData);
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	for (unsigned int n=0; n<height; ++n) delete[] imgData[n];
	delete[] imgData;

	return;

}