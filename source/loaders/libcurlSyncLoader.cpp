// Sync loader based on Libcurl's easy routine
#include "loaders.hpp"
#include <vector>
#include "../utils/Utils.hpp"
#include "../utils/GTLExceptions.hpp"
#include "../external/libcurl/curl/curl.h"

namespace GTLLoaders
{

	size_t readFunc(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		std::vector<GTLUtils::byte> * data = reinterpret_cast<std::vector<GTLUtils::byte> *>(stream);

		std::copy(reinterpret_cast<byte*>(ptr), reinterpret_cast<byte*>(ptr) + (size*nmemb),std::back_inserter(*data));
		return size*nmemb;
	}

	GameTextureLoader3::loaderData libCurlSyncLoader(std::string const &filename)
	{
		CURL * curl = curl_easy_init();
		char curlerror[CURL_ERROR_SIZE];

		std::vector<GTLUtils::byte> tmpdata;
		curl_easy_setopt(curl,CURLOPT_WRITEDATA, &tmpdata);
		curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, readFunc);
		curl_easy_setopt(curl,CURLOPT_URL, filename.c_str());
		curl_easy_setopt(curl,CURLOPT_ERRORBUFFER,curlerror);
		CURLcode rtn = curl_easy_perform(curl);

		if(rtn != 0)
		{
			std::string message(curlerror);
			throw GTLExceptions::IOException(message);
		}
		GameTextureLoader3::DecoderImageData data(new GameTextureLoader3::DecoderImageData::element_type[tmpdata.size()]);
		std::copy(&tmpdata[0],&tmpdata[0]+tmpdata.size(),data.get());
		curl_easy_cleanup(curl);

		return GameTextureLoader3::loaderData(data,tmpdata.size());
	}
}