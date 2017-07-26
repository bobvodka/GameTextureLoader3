// Definitions required for user side decoders
#ifndef GTL_GTLUSERDECODER_HPP
#define GTL_GTLUSERDECODER_HPP

#include "GameTextureLoader.hpp"
#include "IDecoderBase.hpp"
#include <string>
#include <vector>
#include <map>
#include <boost/function.hpp>

namespace GameTextureLoader3
{
	typedef boost::function<Decoders::IDecoderBasePtr (GameTextureLoader3::loaderData, ImgOrigin, ImagePtr)> DecoderCreator_t;

	struct UserDecoderDetails
	{
		std::string extension;
		DecoderCreator_t decoderCreator;
	};

	typedef std::vector<GameTextureLoader3::UserDecoderDetails> userDecoderDetailsVec_t;
	typedef std::map<std::string, GameTextureLoader3::FileTypes> extensionToFileTypesMap_t;

	GameTextureLoader3::extensionToFileTypesMap_t Initalise(int workerThreads, userDecoderDetailsVec_t userDecoders, ImgOrigin origin = ORIGIN_TOP_RIGHT );
}

#endif