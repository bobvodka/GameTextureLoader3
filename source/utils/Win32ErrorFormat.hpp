#ifndef GTL_WIN32ERRORFORMAT_HPP
#define GTL_WIN32ERRORFORMAT_HPP
#include <string>

namespace GTLUtils 
{
	void FormatErrorMessage(int error, std::string &message);
}


#endif