// Win32 specific Async loader function
#define _SCL_SECURE_NO_WARNINGS
#include "loaders.hpp"
#include "../utils/GTLExceptions.hpp"
#include "../utils/Win32ErrorFormat.hpp"
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

namespace GTLLoaders
{
	GameTextureLoader3::loaderData DefaultAsyncLoader(std::string const &filename)
	{
//		std::vector<wchar_t> wfilename(filename.length()+1,L'0');
		size_t rtn;
//		mbstowcs_s(&rtn,&wfilename[0],filename.length()+1,filename.c_str(),filename.size());

		std::wstring wfilename(filename.begin(),filename.end());
		HANDLE file = CreateFile(&wfilename[0],GENERIC_READ,FILE_SHARE_READ,NULL,
			OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL| FILE_FLAG_NO_BUFFERING |FILE_FLAG_OVERLAPPED|FILE_FLAG_SEQUENTIAL_SCAN,NULL);

		if (file == INVALID_HANDLE_VALUE)
		{
			std::string message;
			GTLUtils::FormatErrorMessage(GetLastError(),message);
			throw GTLExceptions::IOException(message);
		}

		DWORD size;
		DWORD truesize = GetFileSize(file,&size);
		
		GameTextureLoader3::DecoderImageData data(new GameTextureLoader3::DecoderImageData::element_type[truesize]);
		
		// Due to the CreateFile flags we've used we need to work out the size of the memory block to reserve
		// which is a multiple of a number of bytes in a sector. This lets the NO_BUFFERING flag above work properly
		// as our reads have to be multiples of sector byte counts.
		DWORD sectorspercluster, bytespersector,numfreeclusters,totalclusters;
		GetDiskFreeSpace(NULL, &sectorspercluster,&bytespersector,&numfreeclusters,&totalclusters);
		DWORD memsize = 0;
		while(memsize < truesize) memsize += bytespersector;

		// Need to Virtual Alloc some memory as we need to read into page aligned memory when doing Async io.
		unsigned char * memblock = static_cast<unsigned char*>(VirtualAlloc(0,memsize,MEM_COMMIT,PAGE_READWRITE));

		if(memblock == NULL)
		{
			std::string message;
			GTLUtils::FormatErrorMessage(GetLastError(),message);
			throw GTLExceptions::IOException(message);
		}

		HANDLE readFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		OVERLAPPED readOverlapped;
		ZeroMemory(&readOverlapped,sizeof(OVERLAPPED));
		readOverlapped.hEvent = readFinishedEvent;
		
		BOOL readFileRtn = ReadFile(file,memblock,memsize,NULL,&readOverlapped);
		DWORD lasterror = GetLastError();
		if(lasterror != ERROR_IO_PENDING)
		{	

			CloseHandle(file);
			CloseHandle(readFinishedEvent);

			std::string message;
			GTLUtils::FormatErrorMessage(lasterror,message);
			throw GTLExceptions::IOException(message);
		}

		DWORD wait = WaitForSingleObject(readOverlapped.hEvent,INFINITE);
	
		std::string message;
		if(wait != WAIT_FAILED)
		{
			// Copy into our real memory block
			std::copy(memblock,memblock+truesize,data.get());
		}
		else
		{
			GTLUtils::FormatErrorMessage(GetLastError(),message);
		}

		// and free the memory
		VirtualFree(memblock,memsize,MEM_COMMIT);

		CloseHandle(file);
		CloseHandle(readFinishedEvent);

		if(wait == WAIT_FAILED)
		{
			throw GTLExceptions::IOException(message);
		}
	
		return GameTextureLoader3::loaderData(data,truesize);
	}
}
