#define _SCL_SECURE_NO_WARNINGS
#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../core/gtlcore.hpp"
#include "ImageCopiers.hpp"
#include "Utils.hpp"
#include "memoryIterators.hpp"

#include <fstream>

namespace GTLImageCopiers
{
	// DXT1 lookup table flipping routines

	GTLUtils::dxt1 fullFlip(GTLUtils::dxt1 &src)
	{
		GTLUtils::dxt1 dest;
		dest.colour0 = src.colour0;
		dest.colour1 = src.colour1;

		boost::uint32_t srcmask = 0x3;			// bit mask for copying (3 = lower 2 bits on)
		dest.lookuptable = fullFlipLookup(src.lookuptable,srcmask);
		return dest;
	}

	GTLUtils::dxt1 yFlip(GTLUtils::dxt1 &src)
	{
		GTLUtils::dxt1 dest;
		dest.colour0 = src.colour0;
		dest.colour1 = src.colour1;

		boost::uint32_t srcmask = 0xFF;		// bit mask for copying (lower 8 bits on)
		dest.lookuptable = yFlipLookup(src.lookuptable,srcmask);
		return dest;
	}

	GTLUtils::dxt1 xFlip(GTLUtils::dxt1 &src)
	{
		GTLUtils::dxt1 dest;
		dest.colour0 = src.colour0;
		dest.colour1 = src.colour1;

		boost::uint32_t srcmask = 0x3;		// bit mask for copying
		dest.lookuptable = xFlipLookup(src.lookuptable,srcmask);
		return dest;
	}

	GTLUtils::dxt2 fullFlip(GTLUtils::dxt2 &src)
	{
		GTLUtils::dxt2 dest;
		dest.colourdata = fullFlip(src.colourdata);

		boost::uint64_t srcmask = 0xF;
		dest.alphatable = fullFlipLookup(src.alphatable, srcmask);

		return dest;
	}

	GTLUtils::dxt2 yFlip(GTLUtils::dxt2 &src)
	{
		GTLUtils::dxt2 dest;
		dest.colourdata = yFlip(src.colourdata);
		
		boost::uint64_t srcmask = 0xFFFF;
		dest.alphatable = yFlipLookup(src.alphatable, srcmask);

		return dest;
	}

	GTLUtils::dxt2 xFlip(GTLUtils::dxt2 &src)
	{
		GTLUtils::dxt2 dest;
		dest.colourdata = xFlip(src.colourdata);

		boost::uint64_t srcmask = 0xF;
		dest.alphatable = xFlipLookup(src.alphatable, srcmask);

		return dest;
	}

	GTLUtils::dxt4 fullFlip(GTLUtils::dxt4 &src)
	{
		GTLUtils::dxt4 dest;
	
		dest = yFlip(src);
		dest = xFlip(dest);

		return dest;
	}

	
	GTLUtils::dxt4 yFlip(GTLUtils::dxt4 &src)
	{
		GTLUtils::dxt4 dest;
		dest.alpha0 = src.alpha0;
		dest.alpha1 = src.alpha1;

		// Dest = lower half | lower half
		dest.lookup6 = ((0xF & src.lookup1) << 4) | ((0xF0 & src.lookup2) >> 4);
		dest.lookup5 = ((0xF & src.lookup3) << 4) | ((0xF0 & src.lookup1) >> 4);
		dest.lookup4 = ((0xF & src.lookup2) << 4) | ((0xF0 & src.lookup3) >> 4);
		dest.lookup3 = ((0xF & src.lookup4) << 4) | ((0xF0 & src.lookup5) >> 4);
		dest.lookup2 = ((0xF & src.lookup6) << 4) | ((0xF0 & src.lookup4) >> 4);
		dest.lookup1 = ((0xF & src.lookup5) << 4) | ((0xF0 & src.lookup6) >> 4);

		dest.colourdata = yFlip(src.colourdata);
		return dest;
	}

	GTLUtils::dxt4 xFlip(GTLUtils::dxt4 &src)
	{

		GTLUtils::dxt4 dest;
		dest.alpha0 = src.alpha0;
		dest.alpha1 = src.alpha1;
		// row 0
		dest.lookup1 = (0x70 & src.lookup2) << 1;	// pixel 4
		dest.lookup1 |= ( 0x80 & src.lookup2) >> 5; // pixel 3
		dest.lookup1 |= ( 0x3 & src.lookup1) << 3;	// and again
		dest.lookup1 |= (0x1c & src.lookup1) >> 3;	// pixel 2
		dest.lookup2 = (0x1c & src.lookup1) << 5;	// and again
		dest.lookup2 |= (0xE0 & src.lookup1) >> 1;	// pixel 1

		// row 1
		dest.lookup2 |= (0x7 & src.lookup3) << 1;	// pixel 4
		dest.lookup2 |= (0x38 & src.lookup3) >> 5;	// pixel 3
		dest.lookup3 = (0x38 & src.lookup3) << 3;	// and again
		dest.lookup3 |= (0x1 & src.lookup2) << 5;	// pixel 2
		dest.lookup3 |= (0xC0 & src.lookup3) >> 3;	// and again
		dest.lookup3 |= (0xE & src.lookup2) >> 1;	// pixel 4

		// row 2
		dest.lookup4 = (0x70 & src.lookup5) << 1;	// pixel 4
		dest.lookup4 |= ( 0x80 & src.lookup5) >> 5; // pixel 3
		dest.lookup4 |= ( 0x3 & src.lookup4) << 3; // and again
		dest.lookup4 |= (0x1c & src.lookup4) >> 3;	// pixel 2
		dest.lookup5 = (0x1c & src.lookup4) << 5;	// and again
		dest.lookup5 |= (0xE0 & src.lookup4) >> 1;	// pixel 1

		// row 3
		dest.lookup5 |= (0x7 & src.lookup6) << 1;	// pixel 4
		dest.lookup5 |= (0x38 & src.lookup6) >> 5;	// pixel 3
		dest.lookup6 = (0x38 & src.lookup6) << 3;	// and again
		dest.lookup6 |= (0x1 & src.lookup5) << 5;	// pixel 2
		dest.lookup6 |= (0xC0 & src.lookup6) >> 3;	// and again
		dest.lookup6 |= (0xE & src.lookup5) >> 1;	// pixel 4

		dest.colourdata = xFlip(src.colourdata);

		return dest;
	}
}