// Decoder class for PNG data
// switch off warnings for the std::copy and std::transform operations below
#define _SCL_SECURE_NO_WARNINGS

#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../../includes/gtl/IDecoderBase.hpp"
#include "../core/gtlcore.hpp"
#include "../utils/Utils.hpp"
#include "../utils/ImageCopiers.hpp"
#include "../utils/GTLExceptions.hpp"
#include <boost/lexical_cast.hpp>

#include <cstdio>  // needed by jpeglib.h
#include <csetjmp>

extern "C" {  // stupid JPEG library
#ifdef WIN32
#include "../external/jpeg-6b/jpeglib.h"
#else
#include <jpeglib.h>
#endif
}

namespace Decoders
{
	namespace JPG
	{
		struct InternalStruct 
		{
			struct {
				jpeg_error_mgr mgr;
				jmp_buf setjmp_buffer;
			} error_mgr;

			GTLUtils::byte * src;
			GTLUtils::byte * buffer;
			int srcdatasize;
			int offset;
		};

		static const int JPEG_BUFFER_SIZE = 4096;

		void init_source(j_decompress_ptr cinfo) {
			// no initialization required
		}

		//////////////////////////////////////////////////////////////////////////////

		boolean fill_input_buffer(j_decompress_ptr cinfo) {
			// more or less copied from jdatasrc.c

			JPG::InternalStruct* is = (JPG::InternalStruct*)(cinfo->client_data);

			int copyBytes = JPEG_BUFFER_SIZE < (is->srcdatasize - is->offset) ? JPEG_BUFFER_SIZE : (is->srcdatasize-is->offset);
			if (copyBytes > 0)
			{
				GTLUtils::byte* src = is->src + is->offset;
				std::copy(src,src + copyBytes,is->buffer);
			}
			else
			{
				is->buffer[0] = (JOCTET)0xFF;
				is->buffer[1] = (JOCTET)JPEG_EOI;
				copyBytes = 2;
			}
					
			cinfo->src->bytes_in_buffer = copyBytes;
			cinfo->src->next_input_byte = is->buffer;
			is->offset += copyBytes;

			return TRUE;
		}

		//////////////////////////////////////////////////////////////////////////////

		void skip_input_data(j_decompress_ptr cinfo, long num_bytes) {
			if (num_bytes > 0) {
				while (num_bytes > (long)cinfo->src->bytes_in_buffer) {
					num_bytes -= (long)cinfo->src->bytes_in_buffer;
					fill_input_buffer(cinfo);
				}
				cinfo->src->next_input_byte += (size_t)num_bytes;
				cinfo->src->bytes_in_buffer -= (size_t)num_bytes;
			}
		}

		//////////////////////////////////////////////////////////////////////////////

		void term_source(j_decompress_ptr cinfo) {
			// nothing to do here...
		}

		//////////////////////////////////////////////////////////////////////////////

		void error_exit(j_common_ptr cinfo) {
			JPG::InternalStruct* is = (JPG::InternalStruct*)(cinfo->client_data);
			longjmp(is->error_mgr.setjmp_buffer, 1);
		}

		//////////////////////////////////////////////////////////////////////////////

		void emit_message(j_common_ptr /*cinfo*/, int /*msg_level*/) {
			// ignore error messages
		}
	}

	class JPGDecoder : public IDecoderBase
	{
	public:
		JPGDecoder(GameTextureLoader3::loaderData data, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image)
			: IDecoderBase(data,origin,image)
		{

		}

		void DecodeImpl();
	protected:
	private:
	};

	void JPGDecoder::DecodeImpl()
	{

		using namespace GameTextureLoader3;

		GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
//		img->state_ = GameTextureLoader3::PROCESSING;

		jpeg_source_mgr mgr_;
		mgr_.bytes_in_buffer = 0;
		mgr_.next_input_byte = NULL;
		mgr_.init_source       = JPG::init_source;
		mgr_.fill_input_buffer = JPG::fill_input_buffer;
		mgr_.skip_input_data   = JPG::skip_input_data;
		mgr_.resync_to_restart = jpeg_resync_to_restart;  // use default
		mgr_.term_source       = JPG::term_source;

		jpeg_decompress_struct cinfo_;
		jpeg_create_decompress(&cinfo_);

		JPG::InternalStruct is_;
		GTLUtils::byte buffer_[JPG::JPEG_BUFFER_SIZE];
		is_.buffer = buffer_;
		is_.src = data.get();
		is_.srcdatasize = datasize;
		is_.offset = 0;

		cinfo_.client_data = &is_;
		cinfo_.err = jpeg_std_error(&is_.error_mgr.mgr);
		is_.error_mgr.mgr.error_exit = JPG::error_exit;

		if (setjmp(is_.error_mgr.setjmp_buffer)) 
		{
			jpeg_destroy_decompress(&cinfo_);
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("JPG: Loading failed.");
		}

		cinfo_.src = &mgr_;

		jpeg_read_header(&cinfo_, TRUE);
		cinfo_.output_components=3;
		cinfo_.out_color_space=JCS_RGB;
		jpeg_start_decompress(&cinfo_);

		if (cinfo_.output_components != 1 && cinfo_.output_components != 3) {
			jpeg_finish_decompress(&cinfo_);
			jpeg_destroy_decompress(&cinfo_);
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("JPG: Unsupported number of components : " +  boost::lexical_cast<std::string>(cinfo_.output_components));
		}

		int row_stride_ = cinfo_.output_width * cinfo_.output_components;
		JSAMPARRAY linebuffer_ = (*cinfo_.mem->alloc_sarray)(
			(j_common_ptr)&cinfo_,
			JPOOL_IMAGE,
			row_stride_,
			1);

		img->width_ = cinfo_.output_width;
		img->height_ = cinfo_.output_height;
		img->numImages_ = 1;
		img->numMipMaps_ = 1;
		int size = cinfo_.output_width * cinfo_.output_height * cinfo_.output_components;
		img->colourdepth_ = cinfo_.output_components * 8;
		if(3 == cinfo_.output_components)
			img->format_ = FORMAT_RGB;
		else
			img->format_ = FORMAT_NONE;	// fix this for grayscale images
		img->depth_ = 0;
		
		if (0==cinfo_.global_state)
		{
//			img->state_ = GameTextureLoader3::ERROR;
//			return;
			throw GTLExceptions::DecoderException("JPG: Invalid global state");
		}

		img->imgdata_.reset(new GTLUtils::byte[size]);
//		GTLUtils::byte * dest = reinterpret_cast<GTLUtils::byte*>(img->imgdata_.get());
		GTLUtils::flipResult flips = GTLUtils::CheckFlips(origin,false,false);

		while (cinfo_.output_scanline < cinfo_.output_height)
		{
			if(jpeg_read_scanlines(&cinfo_,linebuffer_,1) == 1)
			{
/*				GTLUtils::byte * bufferptr = reinterpret_cast<GTLUtils::byte*>(*linebuffer_);	
				std::copy(bufferptr,bufferptr+row_stride_,dest);
				dest += row_stride_;
*/
				GTLUtils::rgb * src = reinterpret_cast<GTLUtils::rgb*>(*linebuffer_);
				GTLImageCopiers::flipRowAndCopy(flips,src,img,cinfo_.output_scanline - 1);
			}
			else
			{
// 				img->state_ = GameTextureLoader3::ERROR;
// 				return;
				throw GTLExceptions::DecoderException("JPG: Error reading scan line data.");
			}
		}

//		jpeg_finish_decompress(&cinfo_);
//		jpeg_destroy_decompress(&cinfo_);

//		img->state_ = GameTextureLoader3::COMPLETED;
	}

	Decoders::IDecoderBasePtr CreateJPGDecoder(GameTextureLoader3::loaderData data, 
		GameTextureLoader3::ImgOrigin origin,GameTextureLoader3::ImagePtr image)
	{
		return IDecoderBasePtr(new JPGDecoder(data,origin,image));
	}
}