
#include "../../includes/gtl/GameTextureLoader.hpp"
#include <boost/cstdint.hpp>

#include "Utils.hpp"


namespace GTLUtils
{
	using namespace GameTextureLoader3;

	typedef unsigned char byte;

	boost::uint16_t read16_le(const byte* b) 
	{
		return b[0] + (b[1] << 8);
	}

	void write16_le(byte* b, boost::uint16_t value) 
	{
		b[0] = value & 0xFF;
		b[1] = value >> 8;
	}

	boost::uint16_t read16_be(const byte* b) 
	{
		return (b[0] << 8) + b[1];
	}

	void write16_be(byte* b, boost::uint16_t value) 
	{
		b[0] = value >> 8;
		b[1] = value & 0xFF;
	}

	boost::uint32_t read32_le(const byte* b) 
	{
		return read16_le(b) + (read16_le(b + 2) << 16);
	}

	boost::uint32_t read32_be(const byte* b) 
	{
		return (read16_be(b) << 16) + read16_be(b + 2);
	}

	// count the number of consecutive zeroes on the right side of a
	// binary number
	// 0x00F0 will return 4
	int count_right_zeroes(boost::uint32_t n) 
	{
		int total = 0;
		boost::uint32_t c = 1;
		while ((total < 32) && ((n & c) == 0)) 
		{
			c <<= 1;
			++total;
		}
		return total;
	}

	// count the number of ones in a binary number
	// 0x00F1 will return 5
	int count_ones(boost::uint32_t n) 
	{
		int total = 0;
		boost::uint32_t c = 1;
		for (int i = 0; i < 32; ++i) 
		{
			if (n & c) 
			{
				++total;
			}
			c <<= 1;
		}
		return total;
	}

	int getMinSize(ImgFormat flag)
	{
		int minsize = 1;

		switch(flag) 
		{
		case FORMAT_DXT1:
			minsize = 8;
			break;
		case FORMAT_DXT2:
		case FORMAT_DXT3:
		case FORMAT_DXT4:
		case FORMAT_DXT5:
		case FORMAT_3DC:
			minsize = 16;
			break;
		case FORMAT_NONE:
			minsize = 0;
		default:
			break;
		}
		return minsize;

	}

	int GetMipLevelSize( unsigned int width, unsigned int height, unsigned int depth, ImgFormat format)
	{
		if (!depth)
			depth=1;

		int numPixels=width*height*depth;

		switch( format)
		{
		case FORMAT_L8:
		case FORMAT_A8:
			return numPixels;

		case FORMAT_R16F:
		case FORMAT_R5G6B5:
		case FORMAT_X1R5G5B5:
		case FORMAT_A1R5G5B5:
		case FORMAT_A8L8:
		case FORMAT_L16:
		case FORMAT_V8U8:
			return numPixels*2;

		case FORMAT_RGB:
		case FORMAT_BGR:
			return numPixels*3;

		case FORMAT_RGBA:
		case FORMAT_BGRA:
		case FORMAT_ABGR:
		case FORMAT_R32F:
		case FORMAT_G16R16F:
		case FORMAT_V16U16:
		case FORMAT_G16R16:
		case FORMAT_Q8W8V8U8:
			return numPixels*4;

		case FORMAT_R16G16B16A16F:
		case FORMAT_G32R32F:
			return numPixels*8;

		case FORMAT_R32G32B32A32F:
			return numPixels*16;

		case FORMAT_DXT1:
			return ((width+3)/4) * ((height+3)/4) * depth * 8;
		case FORMAT_DXT2:
		case FORMAT_DXT3:
		case FORMAT_DXT4:
		case FORMAT_DXT5:
		case FORMAT_3DC:
			return ((width+3)/4) * ((height+3)/4) * depth * 16;
		}
		return -1;
	}

	std::string ExtractFileExtension(std::string const &filename)
	{
		std::string ext;
		std::string::size_type offset = filename.rfind('.');
		if( offset != std::string::npos)
		{
			ext = filename.substr(offset+1,filename.length() - offset);
		}
		else
			throw std::runtime_error("Bad Filename");

		std::transform(ext.begin(),ext.end(),ext.begin(),toupper);

		return ext;
	}

	GTLUtils::flipResult CheckFlips(GameTextureLoader3::ImgOrigin origin, bool xflip, bool yflip)
	{
		bool flipx, flipy;
		switch(origin)
		{
		case ORIGIN_TOP_LEFT:
			flipx = false;
			flipy = true;
			break;
		case ORIGIN_BOTTOM_LEFT:
			flipx = false;
			flipy = false;
			break;
		case ORIGIN_TOP_RIGHT:
			flipx = true;
			flipy = true;
			break;
		case ORIGIN_BOTTOM_RIGHT:
			flipx = true;
			flipy = false;
			break;
		default:
			break;
		}
		
		return GTLUtils::flipResult(flipx ? xflip : !xflip, flipy ? yflip : !yflip);
	}

	GTLUtils::rgba convertBGRAtoRGBA(const GTLUtils::rgba & val)
	{
		GTLUtils::rgba rtn = val;
		std::swap(rtn.r, rtn.b);
		return rtn;
	}
}