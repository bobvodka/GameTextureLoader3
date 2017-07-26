// Header file to define the creation functions for the various default decoders
#ifndef GTL_DEFAULTGTLDECODERS_HPP
#define GTL_DEFAULTGTLDECODERS_HPP
#include "../../includes/gtl/GameTextureLoader.hpp"
namespace Decoders
{
	Decoders::IDecoderBasePtr CreateBitmapDecoder(GameTextureLoader3::loaderData, 
		GameTextureLoader3::ImgOrigin, GameTextureLoader3::ImagePtr);

	Decoders::IDecoderBasePtr CreateTGADecoder(GameTextureLoader3::loaderData, 
		GameTextureLoader3::ImgOrigin, GameTextureLoader3::ImagePtr);

	Decoders::IDecoderBasePtr CreateDDSDecoder(GameTextureLoader3::loaderData, 
		GameTextureLoader3::ImgOrigin, GameTextureLoader3::ImagePtr);

	Decoders::IDecoderBasePtr CreatePNGDecoder(GameTextureLoader3::loaderData, 
		GameTextureLoader3::ImgOrigin, GameTextureLoader3::ImagePtr);

	Decoders::IDecoderBasePtr CreateJPGDecoder(GameTextureLoader3::loaderData, 
		GameTextureLoader3::ImgOrigin, GameTextureLoader3::ImagePtr);
	
}
#endif