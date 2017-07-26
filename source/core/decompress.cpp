#include "gtlcore.hpp"
#include <boost/cstdint.hpp>

namespace GameTextureLoader3
{
	// DXT decoding functionality
	int getChannelCount(GameTextureLoader3::ImgFormat format)
	{
		int rtn = 0;
		switch(format)
		{
		case GameTextureLoader3::FORMAT_DXT1:
			rtn = 3;
			break;
		case GameTextureLoader3::FORMAT_DXT3:
		case GameTextureLoader3::FORMAT_DXT5:
			rtn = 4;
			break;
		case GameTextureLoader3::FORMAT_3DC:
			rtn = 1;
			break;
		default:
			break;
		}
		return rtn;
	}

	void decodeColorBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, const GameTextureLoader3::ImgFormat format, unsigned char *src)
	{
		unsigned char colors[4][3];

		unsigned short c0 = *(unsigned short *) src;
		unsigned short c1 = *(unsigned short *) (src + 2);

		// Extract the two stored colors
		colors[0][0] = ((c0 >> 11) & 0x1F) << 3;
		colors[0][1] = ((c0 >>  5) & 0x3F) << 2;
		colors[0][2] =  (c0        & 0x1F) << 3;

		colors[1][0] = ((c1 >> 11) & 0x1F) << 3;
		colors[1][1] = ((c1 >>  5) & 0x3F) << 2;
		colors[1][2] =  (c1        & 0x1F) << 3;

		// Compute the other two colors
		if (c0 > c1 || format == GameTextureLoader3::FORMAT_DXT5){
			for (int i = 0; i < 3; i++){
				colors[2][i] = (2 * colors[0][i] +     colors[1][i] + 1) / 3;
				colors[3][i] = (    colors[0][i] + 2 * colors[1][i] + 1) / 3;
			}
		} else {
			for (int i = 0; i < 3; i++){
				colors[2][i] = (colors[0][i] + colors[1][i] + 1) >> 1;
				colors[3][i] = 0;
			}
		}

		src += 4;
		for (int y = 0; y < h; y++){
			unsigned char *dst = dest + yOff * y;
			unsigned int indexes = src[y];
			for (int x = 0; x < w; x++){
				unsigned int index = indexes & 0x3;
				dst[0] = colors[index][0];
				dst[1] = colors[index][1];
				dst[2] = colors[index][2];
				indexes >>= 2;

				dst += xOff;
			}
		}
	}

	void decodeDXT3AlphaBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, unsigned char *src)
	{
		for (int y = 0; y < h; y++){
			unsigned char *dst = dest + yOff * y;
			unsigned int alpha = ((unsigned short *) src)[y];
			for (int x = 0; x < w; x++){
				*dst = (alpha & 0xF) * 17;
				alpha >>= 4;
				dst += xOff;
			}
		}
	}
	void decodeDXT5AlphaBlock(unsigned char *dest, const int w, const int h, const int xOff, const int yOff, unsigned char *src)
	{
		unsigned char a0 = src[0];
		unsigned char a1 = src[1];
		boost::uint64_t alpha = (*(boost::uint64_t *) src) >> 16;

		for (int y = 0; y < h; y++){
			unsigned char *dst = dest + yOff * y;
			for (int x = 0; x < w; x++){
				int k = ((unsigned int) alpha) & 0x7;
				if (k == 0){
					*dst = a0;
				} else if (k == 1){
					*dst = a1;
				} else if (a0 > a1){
					*dst = ((8 - k) * a0 + (k - 1) * a1) / 7;
				} else if (k >= 6){
					*dst = (k == 6)? 0 : 255;
				} else {
					*dst = ((6 - k) * a0 + (k - 1) * a1) / 5;
				}
				alpha >>= 3;

				dst += xOff;
			}
			if (w < 4) alpha >>= (3 * (4 - w));
		}
	}
	void decodeCompressedImage(unsigned char *dest, unsigned char *src, const int width, const int height, const ImgFormat format)
	{
		int sx = (width  < 4)? width  : 4;
		int sy = (height < 4)? height : 4;

		int nChannels = getChannelCount(format);
		for (int y = 0; y < height; y += 4)
		{
			for (int x = 0; x < width; x += 4)
			{
				unsigned char *dst = dest + (y * width + x) * nChannels;
				if (format == GameTextureLoader3::FORMAT_DXT3)
				{
					decodeDXT3AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
					src += 8;
				} else if (format == GameTextureLoader3::FORMAT_DXT5)
				{
					decodeDXT5AlphaBlock(dst + 3, sx, sy, 4, width * 4, src);
					src += 8;
				}
				if (format <= GameTextureLoader3::FORMAT_DXT5)
				{
					decodeColorBlock(dst, sx, sy, nChannels, width * nChannels, format, src);
					src += 8;
				} else {
					if (format == /*FORMAT_ATI1N*/ GameTextureLoader3::FORMAT_3DC)
					{
						decodeDXT5AlphaBlock(dst, sx, sy, 1, width, src);
						src += 8;
					} else {
						decodeDXT5AlphaBlock(dst,     sx, sy, 2, width * 2, src + 8);
						decodeDXT5AlphaBlock(dst + 1, sx, sy, 2, width * 2, src);
						src += 16;
					}
				}
			}
		}
	}
	
	GameTextureLoader3::ImagePtr Decompress(GameTextureLoader3::ImagePtr img)
	{
		// Not a compressed type we know about, so just return a ptr to the original image
		GameTextureLoader3::ImgFormat srcFormat = img->getFormat();
		if (srcFormat > GameTextureLoader3::FORMAT_3DC || srcFormat < FORMAT_DXT1)
			return img;

		GameTextureLoader3::ImgFormat destFormat = (srcFormat == FORMAT_DXT1)? FORMAT_RGB : FORMAT_RGBA;
		int destbpp = (destFormat == FORMAT_RGBA) ? 4 : 3;	// bytes per pixel
	
		const int numImages = img->getNumImages();
		const int numMipMaps = img->getNumMipMaps();

		GameTextureLoader3::ImagePtr dest(new GTLCore::ImageImpl(*dynamic_cast<GTLCore::ImageImpl*>(img.get())));
	
		size_t totalspace = 0;
		for (int i = 0; i < numMipMaps; ++i)
		{
			int width = img->getWidth(i);
			int height = img->getHeight(i);
			int depth = img->getDepth(i);
			int size = width * height * (destbpp);		
			size *= (depth != 0) ? depth : 1;
			totalspace += size;
		}

		totalspace *= numImages;

		GTLCore::ImageImpl * realdest = dynamic_cast<GTLCore::ImageImpl*>(dest.get());
		realdest->format_ = destFormat;
		realdest->colourdepth_ = destbpp * 8;

		realdest->imgdata_.reset(new unsigned char[totalspace]);

		for (int image = 0; image < numImages; ++image)
		{
			for(int mipmap = 0; mipmap < numMipMaps; ++mipmap)
			{
				const int width = img->getWidth(mipmap);
				const int height = img->getHeight(mipmap);
				int depth = img->getDepth(mipmap);
				depth = (depth != 0) ? depth : 1;

				unsigned char * src = img->getDataPtr(mipmap,image);
				unsigned char * dst = dest->getDataPtr(mipmap,image);

				int dstSliceSize = width * height * destbpp;	
				int srcSliceSize = img->getSize(mipmap);	// get how far we have to skip for this mipmap level

				for (int slice = 0; slice < depth; slice++)
				{
					decodeCompressedImage(dst, src, width, height, srcFormat);
					// need to push forward a slice size
					dst += dstSliceSize;
					src += srcSliceSize;
				}
			}
		}

		return dest;
	}
}
