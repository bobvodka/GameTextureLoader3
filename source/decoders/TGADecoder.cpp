// Decoder class for TGA data
// switch off warnings for the std::copy and std::transform operations below
#define _SCL_SECURE_NO_WARNINGS

#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../../includes/gtl/IDecoderBase.hpp"
#include "../core/gtlcore.hpp"
#include "../utils/Utils.hpp"
#include "../utils/ImageCopiers.hpp"
#include "../utils/GTLExceptions.hpp"

namespace Decoders
{
	class TGADecoder : public IDecoderBase
	{
	public:
		TGADecoder(GameTextureLoader3::loaderData data, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image)
			: IDecoderBase(data,origin,image), runlengthEncoded(false), totalheaderlength(0)
		{

		}
		
		void DecodeImpl();

		
	protected:
	private:
		bool runlengthEncoded;
		int totalheaderlength;
		bool readHeader(bool &xflip, bool &yflip)
		{
			totalheaderlength = data[0] < 18 ? 18 : data[0];	// TGA standard header length = 18
			int cm_type = data[1];
			if(cm_type != 0)
				return false;	// palleted images not supported

			int image_type = data[2];
			if(image_type == 0 || image_type == 1 || image_type == 9)
				return false;	// no image data we can deal with

			if(image_type == 10)
				runlengthEncoded = true;

			if(image_type == 11 || image_type == 32 || image_type == 33)
				return false;	// currently no support for properly compressed images, need to work on this.

			int pixel_depth      = data[16];
			int image_descriptor = data[17];

			xflip = (image_descriptor & (1 << 4)) != 0;  // left-to-right?
			yflip  = (image_descriptor & (1 << 5)) == 0;  // bottom-to-top?

			
			GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());

			switch(pixel_depth)
			{
			case 32:
				img->format_ = GameTextureLoader3::FORMAT_BGRA /*GameTextureLoader3::FORMAT_RGBA*/;
				img->numImages_ = 1;
				break;
			case 24:
				img->format_ = GameTextureLoader3::FORMAT_BGR;
				img->numImages_ = 1;
				break;
//			case 16:
//				imgdata_.format = FORMAT_BGR;
//				imgdata_.numImages = 1;
//				break;
			default:
				img->format_ = GameTextureLoader3::FORMAT_NONE;
				img->numImages_ = 0;
				return false;
			}

			img->width_ = GTLUtils::read16_le(&data[12]);
			img->height_ = GTLUtils::read16_le(&data[14]);
			img->colourdepth_ = data[16];
			img->numMipMaps_ = 1;
			img->depth_ = 0;

			return true;
		};

// 		template<typename T>
// 		void copyRepeatedChunk(T &srcdata, T * data, int count)
// 		{
// 			T * srcaddress = reinterpret_cast<T*>(data);
// 			std::fill_n(srcaddress,count,srcdata);
// 		};
// 
// 		template<typename T>
// 		void copydata(T * src, T * dest, int count)
// 		{
// 			std::copy(src, src + count, dest);
// 		};
// 
// 		bool readBGRRunLengthEncodedImage()
// 		{
// 			GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
// 			int pixelsize = img->colourdepth_/8;
// 
// 			signed int currentbyte = 0;
// 			const int size = img->width_ * img->height_ * (img->colourdepth_/8);
// 
// 			GTLUtils::byte * bytedata = data.get() + totalheaderlength;
// 			GTLUtils::bgr * localimg = reinterpret_cast<GTLUtils::bgr*>(img->imgdata_.get());
// 
// 			while(currentbyte < size )
// 			{
// 				GTLUtils::byte chunkheader = *bytedata; bytedata++;
// 				++currentbyte;
// 				
// 				if(chunkheader < 128) // Then the next chunkheader++ blocks of data are unique pixels
// 				{
// 					chunkheader++;
// 					GTLUtils::bgr * localdata = reinterpret_cast<GTLUtils::bgr*>(bytedata);
// 					copydata(localdata,localimg,chunkheader);
// 					
// 					localimg += chunkheader;
// 					bytedata += chunkheader * pixelsize;
// 
// 				}
// 				else	// RLE compressed data, chunkheader -127 gives us the total number of repeats
// 				{
// 					chunkheader -= 127;
// 					GTLUtils::bgr * localdata = reinterpret_cast<GTLUtils::bgr*>(bytedata);
// 										
// 					copyRepeatedChunk(*localdata,localimg,chunkheader);
// 					
// 					localimg += chunkheader;	
// 					bytedata ++;
// 				}
// 
// 				currentbyte += pixelsize * chunkheader;
// 			}
// 			return true;
// 		}
// 		
// 		bool readRGBARunLengthEncodedImage()
// 		{
// 			GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
// 			int pixelsize = img->colourdepth_/8;
// 
// 			signed int currentbyte = 0;
// 			const int size = img->width_ * img->height_ * (img->colourdepth_/8);
// 
// 			GTLUtils::byte * bytedata = reinterpret_cast<GTLUtils::byte*>(data.get() + totalheaderlength);
// 			GTLUtils::rgba * localimg = reinterpret_cast<GTLUtils::rgba*>(img->imgdata_.get());
// 
// 			while(currentbyte < size )
// 			{
// 				GTLUtils::byte chunkheader = *bytedata; bytedata++;
// 				++currentbyte;
// 
// 				if(chunkheader < 128) // Then the next chunkheader++ blocks of data are unique pixels
// 				{
// 					chunkheader++;
// 					GTLUtils::rgba * localdata = reinterpret_cast<GTLUtils::rgba*>(bytedata);
// 					copydata(localdata,localimg,chunkheader);
// 
// 					localimg += chunkheader;
// 					bytedata += chunkheader * pixelsize;
// 
// 				}
// 				else	// RLE compressed data, chunkheader -127 gives us the total number of repeats
// 				{
// 					chunkheader -= 127;
// 					
// 					GTLUtils::rgba * localdata = reinterpret_cast<GTLUtils::rgba*>(bytedata);
// 										
// 					copyRepeatedChunk(*localdata,localimg,chunkheader);
// 					localimg += chunkheader;	
// 					bytedata++;
// 				}
// 
// 				currentbyte += pixelsize * chunkheader;
// 			}
// 			return true;
// 		}
	};

	void TGADecoder::DecodeImpl()
	{
		 
		GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
//		img->state_ = GameTextureLoader3::PROCESSING;

		bool xflip = false;
		bool yflip = false;

		if (!readHeader(xflip, yflip))
		{
//			img->state_ = GameTextureLoader3::ERROR;
//			return;
			throw GTLExceptions::DecoderException("TGA: Header Decode Failed");
		}
		const int size = img->width_ * img->height_ * (img->colourdepth_/8);
		img->imgdata_.reset(new GTLUtils::byte[size]);

		GTLUtils::flipResult flips = GTLUtils::CheckFlips(origin, xflip, yflip);

		if (runlengthEncoded)
		{
			bool rtn = false;
			switch(img->format_)
			{
			case GameTextureLoader3::FORMAT_BGR:
				rtn = GTLImageCopiers::decodeRLEData<GTLUtils::bgr>(image,data,flips,totalheaderlength);
//				rtn = readBGRRunLengthEncodedImage();
				break;
			case GameTextureLoader3::FORMAT_RGBA:
				rtn = GTLImageCopiers::decodeRLEData<GTLUtils::rgba>(image,data,flips,totalheaderlength);
//				rtn = readRGBARunLengthEncodedImage();
				break;
			case GameTextureLoader3::FORMAT_BGRA:
				rtn = GTLImageCopiers::decodeRLEData<GTLUtils::bgra>(image,data,flips,totalheaderlength);
//				rtn = readRGBARunLengthEncodedImage();
				break;
			}
			
			if(!rtn)
			{
// 				img->state_ = GameTextureLoader3::ERROR;
// 				return;
				throw GTLExceptions::DecoderException("TGA: RLE Decoding Failed");
			}

		}
		else if (img->format_ == GameTextureLoader3::FORMAT_RGBA)
		{
			GTLImageCopiers::flipImageAndTransform<GTLUtils::rgba>(flips,data,img,GTLUtils::convertBGRAtoRGBA,totalheaderlength);
		}
		else if (img->format_ == GameTextureLoader3::FORMAT_BGRA)
		{
			GTLImageCopiers::flipImageAndCopy<GTLUtils::bgra>(flips,data,img,totalheaderlength);		
		}
		else
		{
			GTLImageCopiers::flipImageAndCopy<GTLUtils::bgr>(flips,data,img,totalheaderlength);		
		}
//		img->state_ = GameTextureLoader3::COMPLETED;

	}

	Decoders::IDecoderBasePtr CreateTGADecoder(GameTextureLoader3::loaderData data, 
		GameTextureLoader3::ImgOrigin origin,GameTextureLoader3::ImagePtr image)
	{
		return IDecoderBasePtr(new TGADecoder(data,origin,image));
	}

}