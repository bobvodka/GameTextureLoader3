// Loader functions
#ifndef GTL_LOADERS_HPP
#define GTL_LOADERS_HPP
#include "../../includes/gtl/GameTextureLoader.hpp"

namespace GTLLoaders
{
	GameTextureLoader3::LoaderFunction_t selectLoader(std::string const &filename,GameTextureLoader3::LoaderMode async);
}
#endif