// User facing API definitions
#ifndef GTL_GAMETEXTURELOADER_HPP
#define GTL_GAMETEXTURELOADER_HPP
#include <stdexcept>

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>

#include <string>

namespace GameTextureLoader3
{
	enum ImgFormat
	{
		FORMAT_NONE = 0,
		FORMAT_RGB,
		FORMAT_BGR,
		FORMAT_RGBA,
		FORMAT_BGRA,
		FORMAT_ABGR,
		FORMAT_ARGB,
		FORMAT_DXT1,
		FORMAT_DXT2,
		FORMAT_DXT3,
		FORMAT_DXT4,
		FORMAT_DXT5,
		FORMAT_3DC,

		//some more 3DS formats follow
		FORMAT_R32G32B32A32F,	//4 channel fp32
		FORMAT_R16G16B16A16F,	//4 channel fp16
		FORMAT_G16R16F,			//2 channel fp16
		FORMAT_G32R32F,			//2 channel fp32
		FORMAT_R16F,			//1 channel fp16
		FORMAT_R32F,			//1 channel fp16

		//additional formats for dds mainly
		//rgb formats
		FORMAT_R5G6B5,			//16bit
		FORMAT_X1R5G5B5,		//15bit
		FORMAT_A1R5G5B5,		//15bit + 1 bit alpha
		FORMAT_L8,				//luminance 
		FORMAT_A8L8,			//alpha, luminance
		FORMAT_L16,				//luminance 16bit
		FORMAT_A8,				//alpha only
		FORMAT_G16R16,			//?? normal maps? L16A16 in opengl?

		//normal map formats
		FORMAT_V8U8,			//signed format, nv_texture_shader
		FORMAT_V16U16,			//signed, nv_texture_shader
		FORMAT_Q8W8V8U8,		//signed, nv_texture_shader

		// Additional formats for PNG images
		FORMAT_RGBA16,			// RGBA 16bit (not floating point)
		FORMAT_RGB16,			// RGB 16bit (not floating point)
		FORMAT_A16,				// 16bit alpha only
		FORMAT_A16L16			// 16bit alpha and luminance
	};

	const int TYPE_BMP = 1;
	const int TYPE_JPG = 2;
	const int TYPE_TGA = 3;
	const int TYPE_PNG = 4;
	const int TYPE_DDS = 5;

	enum ImgOrigin
	{
		ORIGIN_TOP_LEFT = 0,
		ORIGIN_TOP_RIGHT,
		ORIGIN_BOTTOM_LEFT,
		ORIGIN_BOTTOM_RIGHT
	};

	enum AsyncState
	{
		PENDING,
		LOADING,
		LOADED,
		PROCESSING,
		COMPLETED,
		ERROR
	};

	enum LoaderMode
	{
		SYNC,
		ASYNC
	};

	class DecoderNotFoundException : public std::runtime_error
	{
	public:
		DecoderNotFoundException (const std::string & msg) : std::runtime_error(msg)
		{
		}
	};

	typedef int FileTypes;

	struct Image
	{
		Image()
		{

		};
		Image(const Image &)
		{

		};

		virtual ~Image(){};

		// Access functions
		unsigned char * getDataPtr(int mipmaplvl = 0, int imgnumber = 0);
		int getWidth(int mipmaplvl = 0);
		int getHeight(int mipmaplvl = 0);
		int getDepth(int mipmaplvl = 0);
		int getSize(int mipmaplvl = 0);
		int getNumMipMaps();
		int getNumImages();
		int getColourDepth();
		int getColorDepth();	// For our American friends
		ImgFormat getFormat();

		AsyncState getState();
		std::string getErrorInfo();

	//	void decompress();	// decompress a DDS texture

	private:
		virtual unsigned char * getDataPtrImpl( int mipmaplvl, int imgnumber) = 0;
		virtual int getWidthImpl(int mipmaplvl) = 0;
		virtual int getHeightImpl(int mipmaplvl) = 0;
		virtual int getDepthImpl(int mipmaplvl) = 0;
		virtual int getSizeImpl(int mipmaplvl) = 0;
		virtual int getNumMipMapsImpl() = 0;
		virtual int getNumImagesImpl() = 0;
		virtual int getColourDepthImpl() = 0;
		virtual ImgFormat getFormatImpl() = 0;
		virtual AsyncState getStateImpl() = 0;
		virtual std::string getErrorInfoImpl() = 0;
		//virtual void decompressImpl() = 0;
	};

	typedef boost::shared_ptr<Image> ImagePtr;
	typedef boost::shared_array<unsigned char> DecoderImageData;

	struct loaderData
	{
		loaderData(GameTextureLoader3::DecoderImageData &data, int size) : data(data), size(size) {};
		GameTextureLoader3::DecoderImageData data;
		int size;
	};

	typedef boost::function<loaderData (std::string const &)> LoaderFunction_t;
	typedef boost::function<void (ImagePtr, std::string const &)> GTLCallBack_t;

	void Initalise(int workerThreads, ImgOrigin origin = ORIGIN_TOP_RIGHT );
	void Initalise(int workerThreads, GTLCallBack_t loadedFunc, ImgOrigin origin = ORIGIN_TOP_RIGHT );
	void Initalise(int workerThreads, GTLCallBack_t loadedFunc, GTLCallBack_t errorFunc, ImgOrigin origin = ORIGIN_TOP_RIGHT );
	void ShutDown();
	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, LoaderMode async = SYNC);
	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async = SYNC);
	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, GameTextureLoader3::FileTypes id, LoaderMode async = SYNC);
	GameTextureLoader3::ImagePtr LoadTexture(std::string const &filename, GameTextureLoader3::FileTypes id, 
												GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async = SYNC);
	GameTextureLoader3::ImagePtr LoadTexture(GameTextureLoader3::FileTypes id,GameTextureLoader3::LoaderFunction_t loaderFunction, LoaderMode async = SYNC);

	void SetOrigin(ImgOrigin origin);
	void SetCallBacks(GTLCallBack_t loadedFunc, GTLCallBack_t errorFunc);

	GameTextureLoader3::ImagePtr Decompress(GameTextureLoader3::ImagePtr img);
}
#endif