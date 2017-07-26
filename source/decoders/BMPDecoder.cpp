// Decoder class for bitmap data
#define _SCL_SECURE_NO_WARNINGS

#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../../includes/gtl/IDecoderBase.hpp"
#include "../core/gtlcore.hpp"
#include "../utils/Utils.hpp"
#include "../utils/ImageCopiers.hpp"
#include "../utils/GTLExceptions.hpp"
#include <boost/lexical_cast.hpp>

namespace Decoders
{

	namespace
	{
		const int headersize = 14;
		const int infoheadersize = 24;
		const int totalheaderoffset = headersize + infoheadersize;
	}	

	class BMPDecoder : public IDecoderBase
	{
	public:
		BMPDecoder(GameTextureLoader3::loaderData data, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image)
			: IDecoderBase(data,origin,image), dataoffset(0)
		{

		}

		struct Header {
			bool os2;

			int file_size;
			int data_offset;
			int width;
			int height;
			int bpp;
			int compression;

			int pitch;  // number of bytes in each scanline
			int image_size;
			int palette_size;	//size of the palette in bytes
		};

		void DecodeImpl();
	protected:
	private:
	
		GTLUtils::byte palette_[1024];

		void Bpp32toRGB( GTLUtils::byte * src, GTLUtils::byte * dst, Header & header)
		{
			for (int height=0; height<header.height; ++height)
			{
				GTLUtils::byte * srcRow=src + height*header.pitch;
				GTLUtils::byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					dstRow[0]=srcRow[0];
					dstRow[1]=srcRow[1];
					dstRow[2]=srcRow[2];
					srcRow+=4;
					dstRow+=3;
				}
			}
		}

		void Bpp24toRGB( GTLUtils::byte * src, GTLUtils::byte * dst, Header & header)
		{
			for (int height=0; height<header.height; ++height)
			{
				GTLUtils::byte * srcRow=src + height*header.pitch;
				GTLUtils::byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					dstRow[0]=srcRow[0];
					dstRow[1]=srcRow[1];
					dstRow[2]=srcRow[2];
					srcRow+=3;
					dstRow+=3;
				}
			}
		}

		void Bpp16toRGB( GTLUtils::byte * src, GTLUtils::byte * dst, Header & header)
		{	
			for (int height=0; height<header.height; ++height)
			{
				GTLUtils::byte * srcRow=src + height*header.pitch;
				GTLUtils::byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					const GTLUtils::byte blue=srcRow[0]&0x1F;
					const GTLUtils::byte green=(srcRow[0]>>5) + ((srcRow[1]&0x03)<<3);
					const GTLUtils::byte red=(srcRow[1]>>2)&0x1F;

					dstRow[0]=(GTLUtils::byte)((double)blue*(255.0/31.0) );
					dstRow[1]=(GTLUtils::byte)((double)green*(255.0/31.0) );
					dstRow[2]=(GTLUtils::byte)((double)red*(255.0/31.0) );
					srcRow+=2;
					dstRow+=3;
				}
			}
		}

		void Bpp8toRGB( GTLUtils::byte * src, GTLUtils::byte * dst, Header & header)
		{
			GTLUtils::byte * color=NULL;
			for (int height=0; height<header.height; ++height)
			{
				GTLUtils::byte * srcRow=src + height*header.pitch;
				GTLUtils::byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					color= palette_ + 4*(int)srcRow[width];
					dstRow[0]=color[0];
					dstRow[1]=color[1];
					dstRow[2]=color[2];
					dstRow+=3;
				}
			}
		}

		void Bpp4toRGB( GTLUtils::byte * src, GTLUtils::byte * dst, Header & header)
		{
			GTLUtils::byte * color=NULL;
			for (int height=0; height<header.height; ++height)
			{
				GTLUtils::byte * srcRow=src + height*header.pitch;
				GTLUtils::byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					const int palettePos=4*((srcRow[width/2]>>((1-width%2)*4))&0x0F);
					color= palette_+palettePos;
					dstRow[0]=color[0];
					dstRow[1]=color[1];
					dstRow[2]=color[2];
					dstRow+=3;
				}
			}
		}

		void Bpp1toRGB( GTLUtils::byte * src, GTLUtils::byte * dst, Header & header)
		{
			if (0==header.palette_size)
			{	//fake a palette
				palette_[0]=0;
				palette_[1]=0;
				palette_[2]=0;
				palette_[3]=0;
				palette_[4]=255;
				palette_[5]=255;
				palette_[6]=255;
				palette_[7]=255;
			}			

			GTLUtils::byte * color=NULL;
			for (int height=0; height<header.height; ++height)
			{
				GTLUtils::byte * srcRow=src + height*header.pitch;
				GTLUtils::byte * dstRow=dst+ height * header.width * 3;
				for (int width=0; width<header.width; ++width)
				{
					color= palette_+4* (int)((srcRow[width/8]>>(7-width%8))&1);

					dstRow[0]=color[0];
					dstRow[1]=color[1];
					dstRow[2]=color[2];
					if (height==300)
					{
						dstRow[0]=0;
						dstRow[1]=255;
						dstRow[2]=0;
					}
					dstRow+=3;
				}
			}
		}
		
		

		bool ReadHeader(GameTextureLoader3::DecoderImageData data, Header &bitmapheader)
		{
			if(data[0] != 'B' || data[1] != 'M')
				return false;

			bitmapheader.file_size = GTLUtils::read32_le(data.get() + 2);
			bitmapheader.data_offset = GTLUtils::read32_le(data.get() + 10);

			dataoffset += headersize;

			return true;
		}

		bool ReadInfoHeader(GameTextureLoader3::DecoderImageData data, Header &bitmapheader)
		{
			int size = GTLUtils::read32_le(data.get() + headersize);
			int readHeaderBytes = 4;	// 4 bytes into info header

			int width, height, planes, bpp, compression, image_size;

			if(size < 40)
			{
				// assume OS/2 bitmap
				if (size < 12)
					return false;

				readHeaderBytes += 8;

				bitmapheader.os2 = true;
				width = GTLUtils::read16_le(data.get() + headersize + 4);
				height = GTLUtils::read16_le(data.get() +headersize + 6);
				planes = GTLUtils::read16_le(data.get() + headersize + 8);
				bpp = GTLUtils::read16_le(data.get() + headersize + 10);
				compression = 0;
				image_size = 0;
			}
			else
			{
				readHeaderBytes += 20;

				bitmapheader.os2 = false;
				width = GTLUtils::read32_le(data.get() +headersize + 4);
				height = GTLUtils::read32_le(data.get() +headersize + 8);
				planes = GTLUtils::read16_le(data.get() +headersize + 12);
				bpp = GTLUtils::read16_le(data.get() +headersize + 14);
				compression = GTLUtils::read32_le(data.get() +headersize + 16);
				image_size = GTLUtils::read32_le(data.get() +headersize + 20);
			}

			if(planes != 1)
				return false;

			if (!(1==bpp || 4==bpp || 8==bpp || 16==bpp || 24==bpp || 32==bpp))
			{
				return false;
			}

			int line_size = 0;

			if (0!=compression)
				return false;	//just plain RGB data supported

			line_size = ((width * bpp + 31)/32)*4;	//32 bit aligned
			image_size = line_size * height;

			bitmapheader.width       = width;
			bitmapheader.height      = height;
			bitmapheader.bpp         = bpp;
			bitmapheader.compression = compression;
			bitmapheader.pitch       = line_size;
			bitmapheader.image_size  = image_size;

			//calculate palette size			
			bitmapheader.palette_size=bitmapheader.data_offset-size-14;

			if (bitmapheader.palette_size>0 && bitmapheader.palette_size<=1024)
			{
				int offset = headersize + infoheadersize;
				std::copy(data.get() + offset, data.get() + offset + bitmapheader.palette_size, palette_);
			}

			if (bitmapheader.os2 && bitmapheader.palette_size>0 && bitmapheader.palette_size<=1024)
			{
				GTLUtils::byte tmp[1024];
				memcpy(tmp, palette_, 1024);
				for (int i=0; i<256; ++i)
				{
					palette_[i*4+0]=tmp[i*3+0];
					palette_[i*4+1]=tmp[i*3+1];
					palette_[i*4+2]=tmp[i*3+2];
					palette_[i*4+3]=0;
				}
			}
			
			dataoffset += size;

			return true;

		}

		int dataoffset;

	};

	void BMPDecoder::DecodeImpl()
	{
		Header bitmapheader;

		GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
		img->state_ = GameTextureLoader3::PROCESSING;

		if(!ReadHeader(data,bitmapheader))
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("BMP: Header Decode Failed");
		}
		if(!ReadInfoHeader(data,bitmapheader))
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("BMP: Info Header Decode Failed");
		}

		img->height_ = bitmapheader.height;
		img->width_ = bitmapheader.width;
		img->colourdepth_ = 24;
		img->depth_ = 0;
		img->numImages_ = 1;
		img->numMipMaps_ = 1;

		int size = bitmapheader.width * bitmapheader.height * 3;

		img->imgdata_.reset(new GTLUtils::byte[size]);
		GameTextureLoader3::DecoderImageData imgdata(new GTLUtils::byte[size]);

		switch (bitmapheader.bpp)
		{
		case 32:
			Bpp32toRGB( data.get() + dataoffset , imgdata.get(), bitmapheader);
			break;
		case 24:
			Bpp24toRGB( data.get() + dataoffset , imgdata.get(), bitmapheader);
			break;
		case 16:
			Bpp16toRGB( data.get() + dataoffset , imgdata.get(), bitmapheader);
			break;
		case 8:
			Bpp8toRGB( data.get() + dataoffset , imgdata.get(), bitmapheader);
			break;
		case 4:
			Bpp4toRGB( data.get() + dataoffset , imgdata.get(), bitmapheader);
			break;
		case 1:
			Bpp1toRGB( data.get() + dataoffset , imgdata.get(), bitmapheader);
			break;
		default:
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("BMP: Unknown BPP detected : " + boost::lexical_cast<std::string>(bitmapheader.bpp));
			break;
		};

		img->format_ = GameTextureLoader3::FORMAT_BGR;

		GTLUtils::flipResult flips = GTLUtils::CheckFlips(origin,false,true);
		GTLImageCopiers::flipImageAndCopy<GTLUtils::bgr>(flips,imgdata,img);

//		img->state_ = GameTextureLoader3::COMPLETED;		
	}

	Decoders::IDecoderBasePtr CreateBitmapDecoder(GameTextureLoader3::loaderData data, 
		GameTextureLoader3::ImgOrigin origin,GameTextureLoader3::ImagePtr image)
	{
		return IDecoderBasePtr(new BMPDecoder(data,origin,image));
	}

}