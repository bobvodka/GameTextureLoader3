// Public interface functionality for GTL
#include "../includes/gtl/GameTextureLoader.hpp"
#include "../includes/gtl/GTLUserDecoder.hpp"

#include "./core/gtlcore.hpp"
#include "./core/decoderRegistry.hpp"
#include "./loaders/loaders.hpp"
#include "../../utils/Utils.hpp"

#include "external/libcurl/curl/curl.h"

namespace GameTextureLoader3
{
	using namespace GTLCore;

	GTLCallBack_t loadedFunction;
	GTLCallBack_t errorFunction;

	void Initalise(int workerThreads, GTLCallBack_t loadedFunc, ImgOrigin origin)
	{
		Initalise(workerThreads,origin);
		loadedFunction = loadedFunc;
	}

	void Initalise(int workerThreads, GTLCallBack_t loadedFunc, GTLCallBack_t errorFunc, ImgOrigin origin)
	{
		Initalise(workerThreads,origin);
		loadedFunction = loadedFunc;
		errorFunction = errorFunc;
	}

	void Initalise(int workerThreads, ImgOrigin origin )
	{
		userDecoderDetailsVec_t userDecoders;
		Initalise(workerThreads, userDecoders,origin);
	}

	void ShutDown()
	{
		GTLCore::ShutDownThreads();
	}

	extensionToFileTypesMap_t Initalise(int workerThreads, userDecoderDetailsVec_t userDecoders, ImgOrigin origin)
	{
		ThreadInitalise(workerThreads);
		SetOrigin(origin);
		curl_global_init(CURL_GLOBAL_ALL);
		return RegisterDecoders(userDecoders);
	}

	void SetCallBacks(GTLCallBack_t loadedFunc, GTLCallBack_t errorFunc)
	{
		loadedFunction = loadedFunc;
		errorFunction = errorFunc;
	}

	ImgOrigin globalorigin;

	void SetOrigin(ImgOrigin origin)
	{
		globalorigin = origin;
	}
/*
	WorkPacket * constructWorkPacket(std::string const &filename, DecoderCreator_t decoderCreator, 
		GameTextureLoader3::LoaderFunction_t loaderFunction, GameTextureLoader3::ImagePtr image)
	{
		return new ImgWorkPacket(filename,loaderFunction,decoderCreator,image);
	}
*/
	GameTextureLoader3::ImagePtr SetupWork(std::string const &filename, DecoderCreator_t decoderCreator, 
		GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async)
	{
		GameTextureLoader3::ImagePtr image(new GTLCore::ImageImpl());
		//	WorkPacket *work = constructWorkPacket(filename,  loaderFunction, decoderCreator);
		WorkPacket *work = new ImgWorkPacket(filename,loaderFunction,decoderCreator, globalorigin, image, loadedFunction, errorFunction);

		if(async == SYNC)
		{
			LoadImageSync(work);
		}
		else
		{
			LoadImageAsync(work);
		}
		return image;
	}

	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, LoaderMode async)
	{
//		GameTextureLoader3::LoaderFunction_t loader = GTLLoaders::selectLoader(GTLUtils::ExtractFileExtension(filename),async);
		GameTextureLoader3::LoaderFunction_t loader = GTLLoaders::selectLoader(filename,async);
		return LoadTexture(filename,loader,async);
	}

	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async)
	{
		DecoderCreator_t decoderCreator = getCreator(GTLUtils::ExtractFileExtension(filename));
		return SetupWork(filename, decoderCreator,loaderFunction,async);
	}

	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, GameTextureLoader3::FileTypes id, LoaderMode async)
	{
//		GameTextureLoader3::LoaderFunction_t loader = GTLLoaders::selectLoader(GTLUtils::ExtractFileExtension(filename),async);
		GameTextureLoader3::LoaderFunction_t loader = GTLLoaders::selectLoader(filename,async);
		return LoadTexture(filename, id, loader,async);
	}

	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, GameTextureLoader3::FileTypes id, 
		GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async)
	{
		DecoderCreator_t decoderCreator = getCreator(id);
		return SetupWork(filename, decoderCreator, loaderFunction, async);
	}

	GameTextureLoader3::ImagePtr LoadTexture(GameTextureLoader3::FileTypes id,GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async)
	{
		DecoderCreator_t decoderCreator = getCreator(id);
		return SetupWork("", decoderCreator, loaderFunction, async);
	}

	unsigned char * Image::getDataPtr(int mipmaplvl, int imgnumber) {return getDataPtrImpl(mipmaplvl,imgnumber);};
	int Image::getWidth(int mipmaplvl){return getWidthImpl(mipmaplvl);};
	int Image::getHeight(int mipmaplvl){return getHeightImpl(mipmaplvl);};
	int Image::getDepth(int mipmaplvl){return getDepthImpl(mipmaplvl);};
	int Image::getSize(int mipmaplvl){return getSizeImpl(mipmaplvl);};
	int Image::getNumMipMaps() { return getNumMipMapsImpl();};
	int Image::getNumImages(){return getNumImagesImpl();};
	int Image::getColourDepth(){return getColourDepthImpl();};
	int Image::getColorDepth(){return getColourDepthImpl();};
	ImgFormat Image::getFormat(){return getFormatImpl();};
	AsyncState Image::getState(){return getStateImpl();};
	std::string Image::getErrorInfo(){return getErrorInfoImpl();};
	//void Image::decompress(){return decompressImpl();};
}
