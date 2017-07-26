// Core functionality and types for GTL
#ifndef GTL_GTLCORE_HPP
#define GTL_GTLCORE_HPP
#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../../includes/gtl/GTLUserDecoder.hpp"

namespace GTLCore
{

	struct WorkPacket
	{
		virtual void Process() = 0;
	};

	struct ImgWorkPacket : public WorkPacket
	{
		ImgWorkPacket(std::string name, GameTextureLoader3::LoaderFunction_t loaderFunction, 
			GameTextureLoader3::DecoderCreator_t decoderCreator, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image,
			GameTextureLoader3::GTLCallBack_t loadedFunc, GameTextureLoader3::GTLCallBack_t errorFunc)
			: name(name), loaderFunction(loaderFunction), decoderCreator(decoderCreator), origin(origin), image(image), loaded(loadedFunc), error(errorFunc)
		{

		}
		ImgWorkPacket(ImgWorkPacket const& lhs) : name(lhs.name), loaderFunction(lhs.loaderFunction), decoderCreator(lhs.decoderCreator), image(lhs.image)
		{
		}

		void Process();

		std::string name;
		GameTextureLoader3::LoaderFunction_t loaderFunction;
		GameTextureLoader3::DecoderCreator_t decoderCreator;
		GameTextureLoader3::ImagePtr image;
		GameTextureLoader3::ImgOrigin origin;
		GameTextureLoader3::GTLCallBack_t loaded;
		GameTextureLoader3::GTLCallBack_t error;
	};

	struct ImageImpl : public GameTextureLoader3::Image
	{
		ImageImpl(const ImageImpl &rhs) : height_(rhs.height_), width_(rhs.width_), depth_(rhs.depth_), colourdepth_(rhs.colourdepth_), format_(rhs.format_), numImages_(rhs.numImages_), 
			numMipMaps_(rhs.numMipMaps_),imgdata_(rhs.imgdata_),state_(rhs.state_)
		{

		};
		ImageImpl() : height_(0), width_(0), depth_(0), colourdepth_(0), format_(GameTextureLoader3::FORMAT_NONE), numImages_(0), 
			numMipMaps_(0),imgdata_(NULL),state_(GameTextureLoader3::PENDING)
		{
		};

		virtual unsigned char * getDataPtrImpl(int mipmaplvl,int imgnumber);
		virtual int getWidthImpl(int mipmaplvl);
		virtual int getHeightImpl(int mipmaplvl);
		virtual int getDepthImpl(int mipmaplvl);
		virtual int getSizeImpl(int mipmaplvl);
		virtual int getNumMipMapsImpl();
		virtual int getNumImagesImpl();
		virtual int getColourDepthImpl();
		virtual GameTextureLoader3::ImgFormat getFormatImpl();
		int getMipMapChainSize(int mipmaplvl);
		virtual GameTextureLoader3::AsyncState getStateImpl();
		virtual std::string getErrorInfoImpl();
		//void decompressImpl();

		int height_;
		int width_;
		int depth_;
		int colourdepth_;
		GameTextureLoader3::ImgFormat format_;
		int numImages_;
		int numMipMaps_;
		GameTextureLoader3::DecoderImageData imgdata_;
		GameTextureLoader3::AsyncState state_;
		std::string errortext_;

	};

	void ThreadInitalise(int workerThreads);
	void LoadImageSync(WorkPacket *work);
	void LoadImageAsync(WorkPacket *work);
	void ShutDownThreads();
}


#endif