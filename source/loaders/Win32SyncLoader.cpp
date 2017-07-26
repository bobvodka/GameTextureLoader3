// Win32 specific sync loader
// Original system didn't work, decided to strip the CL/LF for some crazy reason
// So it's back to Win32 IO we go!

#define _SCL_SECURE_NO_WARNINGS
#include "loaders.hpp"
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "../utils/GTLExceptions.hpp"
#include "../utils/Win32ErrorFormat.hpp"

namespace GTLLoaders
{
	GameTextureLoader3::loaderData DefaultSyncLoader(std::string const &filename)
	{
//		std::vector<wchar_t> wfilename(filename.length()+1,L'0');
		size_t rtn;
//		mbstowcs_s(&rtn,&wfilename[0],filename.length()+1,filename.c_str(),filename.size());

		std::wstring wfilename(filename.begin(),filename.end());
 		HANDLE file = CreateFile(wfilename.c_str(),GENERIC_READ,FILE_SHARE_READ,NULL,
 			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL| FILE_FLAG_SEQUENTIAL_SCAN,NULL);


// 		HANDLE file = CreateFile(&wfilename[0],GENERIC_READ,FILE_SHARE_READ,NULL,
// 			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL| FILE_FLAG_SEQUENTIAL_SCAN,NULL);
		
		if (file == INVALID_HANDLE_VALUE)
		{
			std::string message;
			GTLUtils::FormatErrorMessage(GetLastError(),message);
			throw GTLExceptions::IOException(message);
		}


		DWORD size;
		DWORD truesize = GetFileSize(file,&size);

		GameTextureLoader3::DecoderImageData data(new GameTextureLoader3::DecoderImageData::element_type[truesize]);

		BOOL rdfilertn = ReadFile(file,data.get(),truesize,&size,NULL);
		CloseHandle(file);
		
		if(rdfilertn == 0)
		{
			std::string message;
			GTLUtils::FormatErrorMessage(GetLastError(),message);
			
			throw GTLExceptions::IOException(message);
		}
		return GameTextureLoader3::loaderData(data,truesize);
	}
}