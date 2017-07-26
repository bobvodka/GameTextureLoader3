#ifndef GTL_UTILS_HPP
#define GTL_UTILS_HPP
#include <boost/cstdint.hpp>

#include "memoryIterators.hpp"
#include "Half/half.h"

namespace GTLUtils
{
	using namespace GameTextureLoader3;

	typedef unsigned char byte;
	typedef boost::uint16_t word;

	struct bgr { GTLUtils::byte b,g,r; };
	struct rgba { GTLUtils::byte r,g,b,a; };
	struct bgra { GTLUtils::byte b,g,r,a; };
	struct rgb15 { boost::uint16_t data; };
	struct argb15 { boost::uint16_t data; };

	struct rgb {GTLUtils::byte r,g,b;};
	struct lumalpha { GTLUtils::byte l,a; };
	struct lum { GTLUtils::byte l; };

	struct bgr16 { GTLUtils::word b,g,r; };
	struct rgba16 { GTLUtils::word r,g,b,a; };
	struct rgb16 {GTLUtils::word r,g,b;};
	struct lumalpha16 { GTLUtils::word l,a; };
	struct lum16 { GTLUtils::word l; };

	struct rgba32 { float r,g,b,a; };
	struct gr32 { float g,r; };
	struct r32	{ float r; };

	// 16bit per channel floating point format
	struct rgba16f { half r,g,b,a; };
	struct gr16f { half g,r;};
	struct r16f	{ half r;};

	// DDS tag structs
	struct dxt1 { 
		boost::uint16_t colour0, colour1; 
		boost::uint32_t lookuptable; 
		dxt1() : colour1(0), colour0(0), lookuptable(0) {};
	};

	struct dxt2 { 
		boost::uint64_t alphatable; 
		dxt1 colourdata; 
		dxt2() : colourdata(), alphatable(0) {};
	};
	typedef dxt2 dxt3;

	struct dxt4 { 
		boost::uint8_t alpha0, alpha1; 
		boost::uint8_t lookup1,lookup2,lookup3,lookup4,lookup5,lookup6; 
		dxt1 colourdata; 
		dxt4() : colourdata(), alpha0(0), alpha1(0), lookup1(0), lookup2(0), lookup3(0), lookup4(0), lookup5(0), lookup6(0) {};
	};
	typedef dxt4 dxt5;



	boost::uint16_t read16_le(const byte* b);
	void write16_le(byte* b, boost::uint16_t value);
	boost::uint16_t read16_be(const byte* b);
	void write16_be(byte* b, boost::uint16_t value);
	boost::uint32_t read32_le(const byte* b);
	boost::uint32_t read32_be(const byte* b);

	// count the number of consecutive zeroes on the right side of a
	// binary number
	// 0x00F0 will return 4
	int count_right_zeroes(boost::uint32_t n);

	// count the number of ones in a binary number
	// 0x00F1 will return 5
	int count_ones(boost::uint32_t n) ;

	int getMinSize(ImgFormat flag);
	int GetMipLevelSize( unsigned int width, unsigned int height, unsigned int depth, ImgFormat format);
	std::string ExtractFileExtension(std::string const &filename);

	struct flipResult
	{
		flipResult(bool xflip, bool yflip) : xflip(xflip), yflip(yflip) {};
		bool operator==(flipResult const &rhs)
		{
			return (rhs.xflip && xflip) && (rhs.yflip && yflip);
		}
		bool flipBoth() { return xflip && yflip;}
		bool flipx() { return xflip; }
		bool flipy() { return yflip; }
	private:
		bool xflip;
		bool yflip;
	};

	flipResult CheckFlips(GameTextureLoader3::ImgOrigin origin, bool xflip, bool yflip);
	GTLUtils::rgba convertBGRAtoRGBA(const GTLUtils::rgba & val);
}


#endif