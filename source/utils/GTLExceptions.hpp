// Exceptions used within GTL for error reasons
#ifndef GTL_GTLEXCEPTIONS_HPP
#define GTL_GTLEXCEPTIONS_HPP
#include <exception>

namespace GTLExceptions
{
	class IOException : public std::runtime_error
	{
	public:
		explicit IOException(const std::string &whatString) : std::runtime_error(whatString) {};
	protected:
	private:
	};

	class DecoderException : public std::runtime_error
	{
	public:
		explicit DecoderException(const std::string &whatString) : std::runtime_error(whatString) {};
	protected:
	private:
	};
}
#endif