// Decoder class for PNG data
// switch off warnings for the std::copy and std::transform operations below
#define _SCL_SECURE_NO_WARNINGS

#include "../external/lpng128/png.h"

#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../../includes/gtl/IDecoderBase.hpp"
#include "../core/gtlcore.hpp"
#include "../utils/Utils.hpp"
#include "../utils/ImageCopiers.hpp"
#include "../utils/GTLExceptions.hpp"

namespace Decoders
{
	namespace PNG
	{
		// PNG Access functions
		//////////////////////////////////////////////////////////////////////////////

		struct pngReader
		{
			GTLUtils::byte * src;
			int datasize;
			int offset;
		};

		void read_function(png_structp png_ptr,
			png_bytep data,
			png_size_t length) 
		{
			pngReader * srcStruct = reinterpret_cast<pngReader*>(png_get_io_ptr(png_ptr));

			if (srcStruct->offset + length > srcStruct->datasize)
			{
				png_error(png_ptr, "Read error");
			}

			GTLUtils::byte * src = srcStruct->src + srcStruct->offset;
			std::copy(src, src + length, data);
			srcStruct->offset += length;
			
		}	

		//////////////////////////////////////////////////////////////////////////////

		void warning_function(png_structp png_ptr, png_const_charp error) {
			// no warnings
		}

		//////////////////////////////////////////////////////////////////////////////

		void error_function(png_structp png_ptr, png_const_charp warning) {
			// copied from libpng's pngerror.cpp, but without the fprintf
			jmp_buf jmpbuf;
			memcpy(jmpbuf, png_ptr->jmpbuf, sizeof(jmp_buf));
			longjmp(jmpbuf, 1);
		}
	}

	class PNGDecoder : public IDecoderBase
	{
	public:
		PNGDecoder(GameTextureLoader3::loaderData data, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image)
			: IDecoderBase(data,origin,image)
		{

		}

		void DecodeImpl();
	protected:
	private:
	};

	void PNGDecoder::DecodeImpl()
	{
		using namespace GameTextureLoader3;

		GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
//		img->state_ = GameTextureLoader3::PROCESSING;

		
		GTLUtils::byte sig[8];
		std::copy(data.get(), data.get()+8,sig);
		if (png_sig_cmp(data.get(), 0, 8))
		{
//			img->state_ = GameTextureLoader3::ERROR;
//			return;
			throw GTLExceptions::DecoderException("PNG: Signiture reading failed.");
		}
		
		png_structp png_ptr_ = png_create_read_struct(
			PNG_LIBPNG_VER_STRING,
			NULL, NULL, NULL);
		if (!png_ptr_) 
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("PNG: Failed to create read struct.");
		}

		png_infop info_ptr_ = png_create_info_struct(png_ptr_);
		if (!info_ptr_) 
		{
			png_destroy_read_struct(&png_ptr_, NULL, NULL);
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("PNG: Failed to create info struct.");
		}

		// the PNG error function calls longjmp(png_ptr->jmpbuf)
		if (setjmp(png_jmpbuf(png_ptr_))) 
		{
			png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("PNG: Loading Failed");
		}

		// set the error function
		png_set_error_fn(png_ptr_, 0, PNG::error_function, PNG::warning_function);

		PNG::pngReader src;
		src.src = data.get();
		src.offset = 8;
		src.datasize = datasize;
		// read the image
		png_set_read_fn(png_ptr_, &src, PNG::read_function);
		png_set_sig_bytes(png_ptr_, 8);  // we already read 8 bytes for the sig

		// Read in the remains of the header
		png_read_info(png_ptr_, info_ptr_);

		// Next we obtain the information we require
		int bit_depth;
		int colourtype;
		png_uint_32 width, height;
		if(!png_get_IHDR(png_ptr_,info_ptr_,&width, &height,&bit_depth, &colourtype, NULL, NULL, NULL))
		{
			png_destroy_read_struct(&png_ptr_, &info_ptr_, NULL);
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("PNG: Failed to decode IHDR Data.");
		}
		
		// all images are always expanded to 8bit per pixel
		if(bit_depth < 8)
		{
			png_set_packing(png_ptr_);
			bit_depth = 8;
		}

		if(PNG_COLOR_TYPE_PALETTE == colourtype)
		{
			png_set_packing(png_ptr_);		// expand 1,2 & 4bit depths to 8bit
			png_set_expand(png_ptr_);	// convert paletted colours to full RGB colours
			if(png_get_valid(png_ptr_, info_ptr_, PNG_INFO_tRNS))
			{
				png_set_tRNS_to_alpha(png_ptr_);
				png_color_16 *image_background;

				// If the file has a valid 'background' then render pixels to it
				if (png_get_bKGD(png_ptr_, info_ptr_, &image_background))
					png_set_background(png_ptr_, image_background,PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);

				img->format_ = FORMAT_RGBA;
			}
			else
				img->format_ = FORMAT_RGB;
		}
		else
		{
			switch(bit_depth)
			{
			case 8:
				switch(colourtype)
				{
				case PNG_COLOR_TYPE_RGB_ALPHA: img->format_ = FORMAT_RGBA; break;
				case PNG_COLOR_TYPE_RGB: img->format_ = FORMAT_RGB; break;
				case PNG_COLOR_TYPE_GRAY_ALPHA: img->format_ = FORMAT_A8L8; break;
				case PNG_COLOR_TYPE_GRAY: img->format_ = FORMAT_L8; break;
				default: img->format_ = FORMAT_NONE; break;
				}
				break;
			case 16:
				switch(colourtype)
				{
				case PNG_COLOR_TYPE_RGB_ALPHA: img->format_ = FORMAT_RGBA16; break;
				case PNG_COLOR_TYPE_RGB: img->format_ = FORMAT_RGB16; break;
				case PNG_COLOR_TYPE_GRAY_ALPHA: img->format_ = FORMAT_A16L16; break;
				case PNG_COLOR_TYPE_GRAY: img->format_ = FORMAT_L16; break;
				default: img->format_ = FORMAT_NONE; break;
				}
				break;
			default:
				img->format_ = FORMAT_NONE;
			}
		}
		if (img->format_ == FORMAT_NONE)
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("PNG: Error detecting image format.");
		}

		// update the transformations
		png_read_update_info(png_ptr_, info_ptr_);

		int num_channels = png_get_channels(png_ptr_, info_ptr_);
		img->width_ = width;
		img->height_ = height;
		img->colourdepth_ = bit_depth * num_channels;
		img->depth_ = 0;
		img->numImages_ = 1;
		img->numMipMaps_ = 1;
		
		int rowlenght = png_get_rowbytes(png_ptr_, info_ptr_);
		int size = height * rowlenght;

		png_bytep *row_pointers_ = new png_bytep[height];	
		for (int row = 0; row < height; row++)
		{
			row_pointers_[row] = reinterpret_cast<png_bytep>(png_malloc(png_ptr_, rowlenght));
		}
		png_read_image(png_ptr_, row_pointers_);	// finally load in the image

		img->imgdata_.reset(new GTLUtils::byte[size]);

//		GTLUtils::flipResult flips = GTLUtils::CheckFlips(origin,false,true);
		GTLUtils::flipResult flips = GTLUtils::CheckFlips(origin,false,false);
		for(int row = 0; row < height; row++)
		{
			switch(img->format_)
			{
			case FORMAT_RGBA:
				{
					GTLUtils::rgba * src = reinterpret_cast<GTLUtils::rgba*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_RGB:
				{
					GTLUtils::rgb * src = reinterpret_cast<GTLUtils::rgb*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_A8L8:
				{
					GTLUtils::lumalpha * src = reinterpret_cast<GTLUtils::lumalpha*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_L8:
				{
					GTLUtils::lum * src = reinterpret_cast<GTLUtils::lum*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_RGBA16:
				{
					GTLUtils::rgba16 * src = reinterpret_cast<GTLUtils::rgba16*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_RGB16:
				{
					GTLUtils::rgb16 * src = reinterpret_cast<GTLUtils::rgb16*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_A16L16:
				{
					GTLUtils::lumalpha16 * src = reinterpret_cast<GTLUtils::lumalpha16*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;

			case FORMAT_L16:
				{
					GTLUtils::lum16 * src = reinterpret_cast<GTLUtils::lum16*>(row_pointers_[row]);
					GTLImageCopiers::flipRowAndCopy(flips,src,img,row);
				}
				break;
			}
		}
		
		png_read_end(png_ptr_, info_ptr_);
		png_destroy_read_struct(&png_ptr_, &info_ptr_, (png_infopp)NULL);

//		img->state_ = GameTextureLoader3::COMPLETED;
	}

	Decoders::IDecoderBasePtr CreatePNGDecoder(GameTextureLoader3::loaderData data, 
		GameTextureLoader3::ImgOrigin origin,GameTextureLoader3::ImagePtr image)
	{
		return IDecoderBasePtr(new PNGDecoder(data,origin,image));
	}
}