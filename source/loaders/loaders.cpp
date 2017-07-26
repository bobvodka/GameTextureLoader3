// Loaders functions
#define _SCL_SECURE_NO_WARNINGS
#include "loaders.hpp"
#include <vector>
#include <fstream>
#include <iterator>
namespace GTLLoaders
{
	
	GameTextureLoader3::loaderData DefaultSyncLoader(std::string const &filename);
	// Code below will might well do us for non-win32 platforms, needs to be tested however
/*	{
		std::vector<unsigned char> buffer;	
		
		std::ifstream file(filename.c_str(), std::ios::binary);
		std::istreambuf_iterator<unsigned char> src(file);
		std::istreambuf_iterator<unsigned char> eof;

		std::copy(src,eof,std::back_inserter(buffer));

		GameTextureLoader3::DecoderImageData data(new GameTextureLoader3::DecoderImageData::element_type[buffer.size()]);
		std::copy(buffer.begin(),buffer.end(),data.get());

		return GameTextureLoader3::loaderData(data,buffer.size());
	}*/

	GameTextureLoader3::loaderData DefaultAsyncLoader(std::string const &filename);
	GameTextureLoader3::loaderData libCurlSyncLoader(std::string const &filename);
//	GameTextureLoader3::loaderData libCurlASyncLoader(std::string const &filename);

	GameTextureLoader3::LoaderFunction_t selectLoader(std::string const &filename,GameTextureLoader3::LoaderMode async)
	{
		// Select and return a sane loader for a given filename
 		GameTextureLoader3::LoaderFunction_t loader;		
// 		if(filename.find("http://") == 0
// 			|| filename.find("ftp://") == 0)
		if(filename.find("://")!= std::string::npos)
		{	
			// libcurl based loader return
//			if(async == GameTextureLoader3::SYNC)
				loader = libCurlSyncLoader;
// 			else
// 				loader = libCurlASyncLoader;
		}
		else if (async == GameTextureLoader3::ASYNC)
		{
			loader = DefaultAsyncLoader;
		}
		else
		{
			// sync loader function
			loader = DefaultSyncLoader;
		}

		return loader;
	}
}