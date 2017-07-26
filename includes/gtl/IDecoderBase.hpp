// Base Class for all image decoder classes
#ifndef GTL_IMAGEDECODERBASE_HPP
#define GTL_IMAGEDECODERBASE_HPP

#include <boost/shared_ptr.hpp>
#include "GameTextureLoader.hpp"
//#include "GTLUserDecoder.hpp"

namespace Decoders
{
	class IDecoderBase
	{
	public:
		IDecoderBase(GameTextureLoader3::loaderData data, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image)
			: data(data.data), datasize(data.size), origin(origin), image(image) 
		{};
		void Decode() {DecodeImpl();};
	
	private:
		virtual void DecodeImpl() = 0;
	protected:
		GameTextureLoader3::DecoderImageData data;
		int datasize;
		GameTextureLoader3::ImgOrigin origin;
		GameTextureLoader3::ImagePtr image;
	};

	typedef boost::shared_ptr<IDecoderBase> IDecoderBasePtr;
}
#endif