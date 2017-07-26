// Allows formatting of error message from Win32 GetLastError() returns

#include <Windows.h>
#include <string>
#include <boost/lexical_cast.hpp>
namespace GTLUtils
{
	void FormatErrorMessage(int error, std::string &message)
	{
		LPTSTR lpMsgBuf = NULL;
		DWORD Size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);

//		DWORD fme = GetLastError();
		
		if (lpMsgBuf) 
		{
			std::wstring tmp(lpMsgBuf);
			message.assign(tmp.begin(),tmp.end());
			LocalFree(lpMsgBuf);
			if (Size)
				return;
		}
		
		message.assign("Unknown error code.");
	}
}