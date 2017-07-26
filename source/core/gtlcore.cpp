// Core of the GTL library
// Sets up work and sends it for processing

#include "gtlcore.hpp"
#include "decoderRegistry.hpp"
#include "../loaders/loaders.hpp"
#include "../utils/Utils.hpp"
#include "../utils/GTLExceptions.hpp"

namespace GTLCore
{
	using namespace GameTextureLoader3;

	void ImgWorkPacket::Process()
	{
//		ImageImpl *img = reinterpret_cast<ImageImpl*>(image.get());
//		img->state_ = GameTextureLoader3::LOADING;
		try
		{
			ImageImpl *img = reinterpret_cast<ImageImpl*>(image.get());
			img->state_ = GameTextureLoader3::LOADING;
			GameTextureLoader3::loaderData data = loaderFunction(name);			// load all the data in
			img->state_ = GameTextureLoader3::LOADED;
			Decoders::IDecoderBasePtr decoder = decoderCreator(data, origin, image);	// create a decoder for the data
			img->state_ = GameTextureLoader3::PROCESSING;
			decoder->Decode();			// and decode the image
			img->state_ = GameTextureLoader3::COMPLETED;
			if(loaded)
				loaded(image,name);
		}
//		catch (GTLExceptions::IOException &e)
		catch (std::exception &e)
		{
			ImageImpl *img = reinterpret_cast<ImageImpl*>(image.get());
			img->errortext_.assign(e.what());
			img->format_ = GameTextureLoader3::FORMAT_NONE;
			img->state_ = GameTextureLoader3::ERROR;			
			if(error)
				error(image,name);
		}	
//		img->state_ = GameTextureLoader3::LOADED;
	}

	int ImageImpl::getMipMapChainSize(int mipmaplvl)
	{
		if(mipmaplvl == 0)
			return 0;

		if (mipmaplvl < 0 || mipmaplvl > getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()]");

		int size = 0;
		for(int i = 0; i < mipmaplvl; ++i)
			size += getSize(i);

		return size;
	}

	unsigned char * ImageImpl::getDataPtrImpl(int mipmaplvl, int imgnumber)
	{	
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		if (imgnumber < 0 || imgnumber >= getNumImages())
			throw std::out_of_range("imgnumber not in the range [0..getNumImages()-1]");

		unsigned char* dataptr = imgdata_.get();
		for(int i = 0; i < imgnumber; ++i)
			dataptr += getMipMapChainSize(getNumMipMaps());

		dataptr += getMipMapChainSize(mipmaplvl);		
		return dataptr;
	}

	int ImageImpl::getWidthImpl(int mipmaplvl)
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		return std::max(width_ / (1 << mipmaplvl),1);
	}
	int ImageImpl::getHeightImpl(int mipmaplvl)
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		return std::max(height_ / (1 << mipmaplvl), 1);
	}
	int ImageImpl::getDepthImpl(int mipmaplvl)	// Returns the number of slices at this mipmap level
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");
		if(depth_ == 0)
			return depth_;

		return std::max(depth_ / (1 << mipmaplvl),1);
	}
	int ImageImpl::getSizeImpl( int mipmaplvl)
	{
		if (mipmaplvl < 0 || mipmaplvl >= getNumMipMaps())
			throw std::out_of_range("mipmaplvl not in the range [0..getNumMipMaps()-1]");

		//int minsize = Utils::getMinSize(getFormat());
//		int size = 0;
		int width = getWidth(mipmaplvl);
		int height = getHeight(mipmaplvl);
		int depth = getDepth(mipmaplvl);

		return GTLUtils::GetMipLevelSize(width, height, depth, this->format_);
		
	}

	int ImageImpl::getNumMipMapsImpl()
	{
		return numMipMaps_;
	}
	int ImageImpl::getNumImagesImpl()
	{
		return numImages_;
	}
	int ImageImpl::getColourDepthImpl()
	{
		return colourdepth_;
	}
	ImgFormat ImageImpl::getFormatImpl()
	{
		return format_;
	}

	GameTextureLoader3::AsyncState ImageImpl::getStateImpl()
	{
		return state_;
	}

	std::string ImageImpl::getErrorInfoImpl()
	{
		return errortext_;
	}

}


