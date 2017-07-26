// Decoder class for DDS data
// switch off warnings for the std::copy and std::transform operations below
#define _SCL_SECURE_NO_WARNINGS

#include "../../includes/gtl/GameTextureLoader.hpp"
#include "../../includes/gtl/IDecoderBase.hpp"
#include "../core/gtlcore.hpp"
#include "../utils/Utils.hpp"
#include "../utils/ImageCopiers.hpp"
#include "../utils/GTLExceptions.hpp"

// Note: This might cause troubles on windows system with big endian CPUs,
//       if recent windows versions ever get ported to such platform. ;-)
#ifdef WIN32
#define __i386__
#endif

namespace Decoders
{
	#define FOURCC(c0, c1, c2, c3) (c0 | (c1 << 8) | (c2 << 16) | (c3 << 24))

	namespace DDS
	{
		struct DDSStruct
		{
			boost::uint32_t 	size;		// equals size of struct (which is part of the data file!)
			boost::uint32_t	flags;
			boost::uint32_t	height,width;
			boost::uint32_t	sizeorpitch;
			boost::uint32_t	depth;
			boost::uint32_t	mipmapcount;
			boost::uint32_t	reserved[11];
			struct pixelformatstruct
			{
				boost::uint32_t	size;	// equals size of struct (which is part of the data file!)
				boost::uint32_t	flags;
				boost::uint32_t	fourCC;
				boost::uint32_t	RGBBitCount;
				boost::uint32_t	rBitMask;
				boost::uint32_t	gBitMask;
				boost::uint32_t	bBitMask;
				boost::uint32_t	alpahbitmask;
			} pixelformat;
			struct ddscapsstruct
			{
				boost::uint32_t	caps1,caps2;
				boost::uint32_t  reserved[2];
			} ddscaps;
			boost::uint32_t reserved2;
		};

		// DDSStruct Flags
		const boost::int32_t	DDSD_CAPS = 0x00000001;
		const boost::int32_t	DDSD_HEIGHT = 0x00000002;
		const boost::int32_t	DDSD_WIDTH = 0x00000004;
		const boost::int32_t	DDSD_PITCH = 0x00000008;
		const boost::int32_t	DDSD_PIXELFORMAT = 0x00001000;
		const boost::int32_t	DDSD_MIPMAPCOUNT = 0x00020000;
		const boost::int32_t	DDSD_LINEARSIZE = 0x00080000;
		const boost::int32_t	DDSD_DEPTH = 0x00800000;

		// pixelformat values
		const boost::int32_t	DDPF_ALPHAPIXELS = 0x00000001;
		const boost::int32_t	DDPF_FOURCC = 0x00000004;
		const boost::int32_t	DDPF_RGB = 0x00000040;

		// ddscaps
		// caps1
		const boost::int32_t	DDSCAPS_COMPLEX = 0x00000008;
		const boost::int32_t	DDSCAPS_TEXTURE = 0x00001000;
		const boost::int32_t	DDSCAPS_MIPMAP = 0x00400000;
		// caps2
		const boost::int32_t	DDSCAPS2_CUBEMAP = 0x00000200;
		const boost::int32_t	DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
		const boost::int32_t	DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
		const boost::int32_t	DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
		const boost::int32_t	DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
		const boost::int32_t	DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
		const boost::int32_t	DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
		const boost::int32_t	DDSCAPS2_VOLUME = 0x00200000;
	}

	namespace 
	{
		const int ddsheaderSize = 128;
	}

	class DDSDecoder : public IDecoderBase
	{
	public:
		DDSDecoder(GameTextureLoader3::loaderData data, GameTextureLoader3::ImgOrigin origin, GameTextureLoader3::ImagePtr image)
			: IDecoderBase(data,origin,image)
		{

		}

		void DecodeImpl();

	protected:
	private:

		boost::uint32_t ReadDword( GTLUtils::byte * & pData )
		{
			boost::uint32_t value= GTLUtils::read32_le(pData);
			pData+=4;
			return value;
		};

		DDS::DDSStruct surfacedata;

		bool readHeader()
		{
			GTLUtils::byte * pData = data.get();

			if (! (pData[0]=='D' && pData[1]=='D' && pData[2]=='S' && pData[3]==' ') )
			{
				return false;
			}

			pData+=4;

			surfacedata.size = ReadDword(pData);
			if(surfacedata.size != ddsheaderSize - 4)	// 4 bytes we have already read off
			{
				return false;
			}

			//convert the data
			surfacedata.flags=ReadDword(pData);
			surfacedata.height=ReadDword(pData);
			surfacedata.width=ReadDword(pData);
			surfacedata.sizeorpitch=ReadDword(pData);
			surfacedata.depth=ReadDword(pData);
			surfacedata.mipmapcount=ReadDword(pData);

			for (int i=0; i<11; ++i)
			{
				surfacedata.reserved[i]=ReadDword(pData);
			}

			//pixel fromat
			surfacedata.pixelformat.size=ReadDword(pData);
			surfacedata.pixelformat.flags=ReadDword(pData);
			surfacedata.pixelformat.fourCC=ReadDword(pData);
			surfacedata.pixelformat.RGBBitCount=ReadDword(pData);
			surfacedata.pixelformat.rBitMask=ReadDword(pData);
			surfacedata.pixelformat.gBitMask=ReadDword(pData);
			surfacedata.pixelformat.bBitMask=ReadDword(pData);
			surfacedata.pixelformat.alpahbitmask=ReadDword(pData);

			//caps
			surfacedata.ddscaps.caps1=ReadDword(pData);
			surfacedata.ddscaps.caps2=ReadDword(pData);
			surfacedata.ddscaps.reserved[0]=ReadDword(pData);
			surfacedata.ddscaps.reserved[1]=ReadDword(pData);
			surfacedata.reserved2=ReadDword(pData);

			return true;
		};
		
		int calculateStoreageSize()
		{
			int size = 0;
			GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
			for(int i = 0; i < img->numImages_; ++i)
			{
				int width=img->width_;
				int height=img->height_;
				int depth=img->depth_;

				for (int m=0; m<img->numMipMaps_; ++m)
				{
					size+= GTLUtils::GetMipLevelSize(width, height, depth, img->format_);
					width = std::max(width>>1, 1);
					height = std::max(height>>1, 1);
					depth = std::max(depth>>1, 1);
				}
			}

			return size;
		};

		int getNumImages()
		{
			if(!(surfacedata.ddscaps.caps2 & DDS::DDSCAPS2_CUBEMAP))
				return 1;

			// We are a cube map, so work out how many sides we have
			boost::uint32_t mask = DDS::DDSCAPS2_CUBEMAP_POSITIVEX;
			int count = 0;
			for(int n = 0; n < 6; ++n)
			{
				if(surfacedata.ddscaps.caps2 & mask)
					++count;
				mask *= 2;	// move to next face
			}
			return count;		
		};

		GameTextureLoader3::ImgFormat getDXTFormat()
		{
			using namespace GameTextureLoader3; 

			ImgFormat format = FORMAT_NONE;
			switch(surfacedata.pixelformat.fourCC) 
			{
			case FOURCC('D','X','T','1'):
				format = FORMAT_DXT1;
				break;
			case FOURCC('D','X','T','2'):
				format = FORMAT_DXT2;
				break;
			case FOURCC('D','X','T','3'):
				format = FORMAT_DXT3;
				break;
			case FOURCC('D','X','T','4'):
				format = FORMAT_DXT4;
				break;
			case FOURCC('D','X','T','5'):
				format = FORMAT_DXT5;
				break;
			case FOURCC('A','T','I','2'):
				format = FORMAT_3DC;
				break;
			case 0x74:
				format=FORMAT_R32G32B32A32F;
				break;
			case 0x71:
				format=FORMAT_R16G16B16A16F;
				break;
			case 0x70:
				format=FORMAT_G16R16F;
				break;
			case 0x73:
				format=FORMAT_G32R32F;
				break;
			case 0x6F:
				format=FORMAT_R16F;
				break;
			case 0x72:
				format=FORMAT_R32F;
				break;
			default:
				break;
			}
			return format;
		};


		GameTextureLoader3::ImgFormat getTextureFormat()
		{
			using namespace GameTextureLoader3;

			ImgFormat format = FORMAT_NONE;

			if(surfacedata.pixelformat.flags & DDS::DDPF_FOURCC)
			{
				format = getDXTFormat();
			} 
			else if(surfacedata.pixelformat.flags & DDS::DDPF_RGB)
			{
				if(surfacedata.pixelformat.flags & DDS::DDPF_ALPHAPIXELS)
				{
					if (0xff == surfacedata.pixelformat.bBitMask &&
						0xff00 == surfacedata.pixelformat.gBitMask &&
						0xff0000 == surfacedata.pixelformat.rBitMask &&
						0xff000000 == surfacedata.pixelformat.alpahbitmask)
					{
						format = FORMAT_BGRA;
					} else if (	0xff == surfacedata.pixelformat.rBitMask &&
						0xff00 == surfacedata.pixelformat.gBitMask &&
						0xff0000 == surfacedata.pixelformat.bBitMask &&
						0xff000000 == surfacedata.pixelformat.alpahbitmask)
					{
						format = FORMAT_RGBA;
					} else if (	0xff == surfacedata.pixelformat.alpahbitmask &&
						0xff00 == surfacedata.pixelformat.bBitMask &&
						0xff0000 == surfacedata.pixelformat.gBitMask &&
						0xff000000 == surfacedata.pixelformat.rBitMask)
					{
						format = FORMAT_ABGR;
					} else if (	0x8000 == surfacedata.pixelformat.alpahbitmask &&
						0x1F == surfacedata.pixelformat.bBitMask &&
						0x3E0 == surfacedata.pixelformat.gBitMask &&
						0x7C00 == surfacedata.pixelformat.rBitMask)
					{
						format = FORMAT_A1R5G5B5;
					}
				}
				else
				{
					if (0xff == surfacedata.pixelformat.bBitMask &&
						0xff00 == surfacedata.pixelformat.gBitMask &&
						0xff0000 == surfacedata.pixelformat.rBitMask )
					{
						format = FORMAT_BGRA;
					} else if (	0xff == surfacedata.pixelformat.rBitMask &&
						0xff00 == surfacedata.pixelformat.gBitMask &&
						0xff0000 == surfacedata.pixelformat.bBitMask )
					{
						format = FORMAT_RGBA;
					} else if (	0xffFF == surfacedata.pixelformat.rBitMask &&
						0xffFF0000 == surfacedata.pixelformat.gBitMask &&
						0x00 == surfacedata.pixelformat.bBitMask &&
						0x00 == surfacedata.pixelformat.alpahbitmask)
					{
						format = FORMAT_G16R16;
					} else if (	0x1F == surfacedata.pixelformat.bBitMask &&
						0x3E0 == surfacedata.pixelformat.gBitMask &&
						0x7C00 == surfacedata.pixelformat.rBitMask )
					{
						format = FORMAT_X1R5G5B5;
					} else if (	0x1F == surfacedata.pixelformat.bBitMask &&
						0x7E0 == surfacedata.pixelformat.gBitMask &&
						0xF800 == surfacedata.pixelformat.rBitMask )
					{
						format = FORMAT_R5G6B5;
					}
				}
			} else
			{
				if (0xFF==surfacedata.pixelformat.rBitMask &&
					0x0==surfacedata.pixelformat.gBitMask &&
					0x0==surfacedata.pixelformat.bBitMask &&
					0x0==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_L8;
				} else if (	0xFFFF==surfacedata.pixelformat.rBitMask &&
					0x0==surfacedata.pixelformat.gBitMask &&
					0x0==surfacedata.pixelformat.bBitMask &&
					0x0==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_L16;
				} else if (	0x0==surfacedata.pixelformat.rBitMask &&
					0x0==surfacedata.pixelformat.gBitMask &&
					0x0==surfacedata.pixelformat.bBitMask &&
					0xFF==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_A8;
				} else if (	0xFF==surfacedata.pixelformat.rBitMask &&
					0x0==surfacedata.pixelformat.gBitMask &&
					0x0==surfacedata.pixelformat.bBitMask &&
					0xFF00==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_A8L8;
				} else if (	0xFF==surfacedata.pixelformat.rBitMask &&
					0xFF00==surfacedata.pixelformat.gBitMask &&
					0x0==surfacedata.pixelformat.bBitMask &&
					0x0==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_V8U8;
				} else if (	0xFF==surfacedata.pixelformat.rBitMask &&
					0xFF00==surfacedata.pixelformat.gBitMask &&
					0xFF0000==surfacedata.pixelformat.bBitMask &&
					0xFF000000==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_Q8W8V8U8;
				} else if (	0xFFFF==surfacedata.pixelformat.rBitMask &&
					0xFFFF0000==surfacedata.pixelformat.gBitMask &&
					0x0==surfacedata.pixelformat.bBitMask &&
					0x0==surfacedata.pixelformat.alpahbitmask)
				{
					format = FORMAT_V16U16;
				}
			}
			return format;
		};

	};

	void DDSDecoder::DecodeImpl()
	{
		GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
		img->state_ = GameTextureLoader3::PROCESSING;

		if (!readHeader())
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("DDS: Header read failed.");
		}

		if (surfacedata.mipmapcount==0)
			surfacedata.mipmapcount=1;

		img->width_ = surfacedata.width;
		img->height_ = surfacedata.height;

		img->depth_ = surfacedata.flags & DDS::DDSD_DEPTH ? surfacedata.depth : 0;

		img->colourdepth_ = surfacedata.pixelformat.RGBBitCount;
		img->numMipMaps_ = surfacedata.mipmapcount;
		img->format_ = getTextureFormat();
		img->numImages_ = getNumImages();
		int size = calculateStoreageSize();
		if(0 >= size)
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("DDS: Invalid image data size");
		}


		if(GameTextureLoader3::FORMAT_NONE == img->format_)
		{
// 			img->state_ = GameTextureLoader3::ERROR;
// 			return;
			throw GTLExceptions::DecoderException("DDS: Unknown image format");
		}

		img->imgdata_.reset(new GTLUtils::byte[size]);
		
		GTLUtils::flipResult flips = GTLUtils::CheckFlips(origin, false, false);
		switch(img->format_)
		{
		case GameTextureLoader3::FORMAT_DXT1:
			GTLImageCopiers::flipDXTnImage<GTLUtils::dxt1>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_DXT2:
			GTLImageCopiers::flipDXTnImage<GTLUtils::dxt2>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_DXT3:
			GTLImageCopiers::flipDXTnImage<GTLUtils::dxt3>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_DXT4:
			GTLImageCopiers::flipDXTnImage<GTLUtils::dxt4>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_DXT5:
			GTLImageCopiers::flipDXTnImage<GTLUtils::dxt5>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_3DC:
			GTLImageCopiers::flipDXTnImage<GTLUtils::dxt5>(flips,data,img,ddsheaderSize);
			break;
// Non-compressed images
		case GameTextureLoader3::FORMAT_RGB:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::rgb>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_R5G6B5:
		case GameTextureLoader3::FORMAT_X1R5G5B5:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::rgb15>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_A1R5G5B5:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::argb15>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_ABGR:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::rgb>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_ARGB:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::rgb>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_L8:
		case GameTextureLoader3::FORMAT_A8:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::lum>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_L16:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::lum16>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_A8L8:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::lumalpha>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_G16R16:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::lumalpha16>(flips,data,img,ddsheaderSize);
			break;
// floating point formats
		case GameTextureLoader3::FORMAT_R32G32B32A32F:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::rgba32>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_G32R32F:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::gr32>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_R32F:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::r32>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_R16G16B16A16F:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::rgba16f>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_G16R16F:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::gr16f>(flips,data,img,ddsheaderSize);
			break;
		case GameTextureLoader3::FORMAT_R16F:
			GTLImageCopiers::flipImageAndCopy<GTLUtils::r16f>(flips,data,img,ddsheaderSize);
			break;

		default:
			// This needs to go and we need to deal with the various uncompressed DDS formats
// 			GTLUtils::byte * src = data.get() + ddsheaderSize;
// 			GTLUtils::byte * dest = img->imgdata_.get();		
// 			std::copy(src, src + size, dest);
			throw GTLExceptions::DecoderException("DDS: Unknown Image Format at decode stage");
			break;
		}
//		img->state_ = GameTextureLoader3::COMPLETED;
	};

	Decoders::IDecoderBasePtr CreateDDSDecoder(GameTextureLoader3::loaderData data, 
		GameTextureLoader3::ImgOrigin origin,GameTextureLoader3::ImagePtr image)
	{
		return IDecoderBasePtr(new DDSDecoder(data,origin,image));
	}

}