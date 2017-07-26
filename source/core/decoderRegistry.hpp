// Decoder dispatcher definitions
#include <string>
#include "../../includes/gtl/GTLUserDecoder.hpp"
#include "../../includes/gtl/GameTextureLoader.hpp"

namespace GTLCore
{
	// Load up the Decoder Registry with the decoders we know about + any user defined ones
	GameTextureLoader3::extensionToFileTypesMap_t RegisterDecoders(GameTextureLoader3::userDecoderDetailsVec_t const &userdecoders);
	GameTextureLoader3::DecoderCreator_t getCreator(std::string const &ext);
	GameTextureLoader3::DecoderCreator_t getCreator(GameTextureLoader3::FileTypes ext);
}