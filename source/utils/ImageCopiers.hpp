// A series of templated routines to copy image objects between memory buffers
#ifndef GTL_IMAGECOPIERS_HPP
#define GTL_IMAGECOPIERS_HPP

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "Utils.hpp"
#include "memoryIterators.hpp"

//CHECK ALL THE OFFSET MATHS!!!
//IT FEELS LIKE BAD BAD VOODOO!!!
// 

namespace GTLImageCopiers
{
	// confirmed to work (or at least not crash)
	template<class T>
	void flipImageAndCopy(GTLUtils::flipResult flips, 
		GameTextureLoader3::DecoderImageData const &data, GTLCore::ImageImpl *img, int headeroffset = 0)
	{
		using namespace GTLMemoryIterators;
		typedef typename forward_memory_iter<T> forward_mem_iter;
		typedef typename reverse_memory_iter<T> reverse_mem_iter;

		if(flips.flipBoth())
		{
			const int len = img->width_ * img->height_;			
			forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset));
			forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + len);
			reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + len - 1);
			std::copy(src,end,dest);

		}
		else if (flips.flipy())	
		{
			for(int i = 0; i < img->height_; ++i)
			{
//				const int offset = headeroffset + (img->width_ * i * img->colourdepth_/8);
				forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset) + img->width_*i);
				forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + (i+1)*img->width_);

				const int destoffset = img->width_ * img->height_ - ((i+1) * img->width_);
//				reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + destoffset - 1);
				forward_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + destoffset);

				std::copy(src,end,dest);
			}
		}
		else if(flips.flipx())
		{
			for (int i = 0; i < img->height_; ++i)
			{
//				const int offset = headeroffset + (img->width_ * i * img->colourdepth_/8);
				forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset) + img->width_*i);
				forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + (i+1)*img->width_);

				const int destoffset = (img->width_ * (i +1)) - 1;
				reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + destoffset);

				std::copy(src,end,dest);
			}
		}
		else	
		{
			const int len = img->width_ * img->height_;	
			forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset));
			forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + len);
			forward_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()));
			std::copy(src,end,dest);
		}
	}
	
	// confirmed to work (or at least not crash)
	template<class T>
	void flipRowAndCopy(GTLUtils::flipResult flips, T * data, GTLCore::ImageImpl *img, int rowcout )
	{
		using namespace GTLMemoryIterators;
		typedef typename forward_memory_iter<T> forward_mem_iter;
		typedef typename reverse_memory_iter<T> reverse_mem_iter;

		forward_mem_iter src(reinterpret_cast<T*>(data));
		forward_mem_iter end(reinterpret_cast<T*>(data) + img->width_);

		if(flips.flipBoth())
		{
			reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + (img->width_ * (img->height_ - rowcout - 1) + (img->width_ - 1) ));
			std::copy(src,end,dest);
		}
		else if (flips.flipy())	// confirmed to work
		{
			//forward_mem_iter dest(reinterpret_cast<T*>(data) + (img->width_ * (img->height_ - rowcout - 1)));
			const int offset = img->width_ * (img->height_  - rowcout) - img->width_;
			forward_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + offset);
			std::copy(src,end,dest);
		}
		else if(flips.flipx())
		{
			const int offset = (img->width_ * (rowcout +1)) - 1;
			reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + offset /*+ (img->width_ * rowcout) + (img->width_ - 1)*/);
			std::copy(src,end,dest);
		}
		else
		{
			forward_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + (img->width_ * rowcout));
			std::copy(src,end,dest);
		}
	}

	// confirmed to work (or at least not crash)
	template<class T>
	void flipImageAndTransform(GTLUtils::flipResult flips, 
		GameTextureLoader3::DecoderImageData const &data, GTLCore::ImageImpl *img, boost::function<T (T const &)> func, int headeroffset = 0)
	{
		using namespace GTLMemoryIterators;
		typedef typename forward_memory_iter<T> forward_mem_iter;
		typedef typename reverse_memory_iter<T> reverse_mem_iter;

		if(flips.flipBoth())
		{
			const int len = img->width_ * img->height_;			
			forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset));
			forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + len);
			reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + len - 1);
			std::transform(src,end,dest,func);

		}
		else if (flips.flipy())
		{
			for(int i = 0; i < img->height_; ++i)
			{
//				const int offset = headeroffset + (img->width_ * i * img->colourdepth_/8);
				forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset) + img->width_*i);
				forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + img->width_*(i+1));
				const int destoffset = img->width_ * img->height_ - ((i+1) * img->width_);
				forward_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + destoffset);
				std::transform(src,end,dest,func);
			}
		}
		else if(flips.flipx())
		{
			for (int i = 0; i < img->height_; ++i)
			{
//				const int offset = headeroffset + (img->width_ * i * img->colourdepth_/8);
				forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset) + i*img->width_);
				forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + (i+1)*img->width_);
//				const int destoffset = (img->width_ * i) + (img->width_ - 1);
				const int destoffset = (img->width_ * (i +1)) - 1;
				reverse_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()) + destoffset);
				std::transform(src,end,dest,func);
			}
		}
		else
		{
			const int len = img->width_ * img->height_;
			forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset));
			forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset) + len);
			forward_mem_iter dest(reinterpret_cast<T*>(img->imgdata_.get()));
			std::transform(src,end,dest,func);
		}
	}

// DXTn Copying routines

	template<class T>
	T fullFlipLookup(T src, T srcmask)
	{
		T dest = 0;

		const int offset = (sizeof(T) * 8) - (2 * (sizeof(T)/4));	// for 32bit = 30, 64bit = 60
		const int shiftamount = 2 * sizeof(T)/4;					// for 32bit = 2, 64bit = 4

		for (int i = 0; i < 16; i++)
		{
			int tmpshift = offset - (i * shiftamount);
			T tmpdata = src & srcmask;
			// shift to start of bit pattern
			tmpdata =  tmpdata >> (offset - tmpshift);
			// then shift it to the correct location
			T shiftdata = tmpdata << tmpshift;
			dest |= shiftdata;
			srcmask = srcmask << shiftamount;
		}

		return dest;
	}

	template<class T>
	T yFlipLookup(T src, T srcmask)
	{
		T dest = 0;
//		const int offset = (sizeof(T) * 8) - (2 * (sizeof(T)/4));	// for 32bit = 30, 64bit = 60
		const int offset = (sizeof(T) * 8) - (2 * (sizeof(T)));		// for 32bit = 24, 64bit = 48
		const int shiftamount = 2 * sizeof(T)/4;						// for 32bit = 2, 64bit = 4
		const int incamount = sizeof(T);							// for 32bit = 4, 64bit = 8

//		for(int i = 0; i < 16; i+= incamount)
		for(int i = 0; i < 16; i+= 4)	// always moving 4 pixels at a time
		{
			int tmpshift = offset - i*shiftamount;
			T tmpdata = src & srcmask;
			//tmpdata = tmpdata >> (offset - tmpshift); 
			int tmpamount = i*shiftamount;
			tmpdata = tmpdata >> tmpamount; 
			T shiftdata = tmpdata << tmpshift;
			dest |= shiftdata;
			srcmask = srcmask << (incamount*2);
		}
		return dest;
	}

	template<class T>
	T xFlipLookup(T src, T /*original*/srcmask)
	{
		T dest = 0;
		const int xoffset = (sizeof(T)/4) * 6;						// for 32bit = 6, 64bit = 12
		const int yoffset = (sizeof(T)*2);							// for 32bit = 8, 64bit = 16
		const int shiftamount = 2 * sizeof(T)/4;					// for 32bit = 2, 64bit = 4
		//		const int incamount = (sizeof(T)/8) * 4;					// for 32bit = 4, 64bit = 8

		for (int i = 0; i < 4; i++)
		{
			// We don't need to move the mask as we move the src data!
			//T srcmask = (originalsrcmask << (incamount*i*shiftamount));
			for (int j = 0; j < 4; j++)
			{
				T srcbits = src & srcmask;
				T xshift = xoffset - j*shiftamount;
				T yshift = yoffset*i;
				T totalshift = xshift + yshift;
//				dest |= (srcbits << ((xoffset - j*shiftamount) + (yoffset*i)));
				T tmpdata = srcbits << totalshift;
				dest |= tmpdata;
				src = src >> shiftamount;
			}
		}
		return dest;
	}

	GTLUtils::dxt1 fullFlip(GTLUtils::dxt1 &src);
	GTLUtils::dxt1 xFlip(GTLUtils::dxt1 &src);
	GTLUtils::dxt1 yFlip(GTLUtils::dxt1 &src);
	
	GTLUtils::dxt2 fullFlip(GTLUtils::dxt2 &src);
	GTLUtils::dxt2 xFlip(GTLUtils::dxt2 &src);
	GTLUtils::dxt2 yFlip(GTLUtils::dxt2 &src);
	
	GTLUtils::dxt4 fullFlip(GTLUtils::dxt4 &src);
	GTLUtils::dxt4 xFlip(GTLUtils::dxt4 &src);
	GTLUtils::dxt4 yFlip(GTLUtils::dxt4 &src);


	// We need to do the flips for each images and mipmap in the chain
	// 3D textures might have to be ignored until I can think of some sane way to handle it
	template<class T>
	void flipDXTnImage(GTLUtils::flipResult flips, 
		GameTextureLoader3::DecoderImageData const &data, GTLCore::ImageImpl *img, int headeroffset = 0)
	{
		using namespace GTLMemoryIterators;
		typedef typename forward_memory_iter<T> forward_mem_iter;
		typedef typename reverse_memory_iter<T> reverse_mem_iter;

		int realheight = (img->height_ < 16) ? img->height_ : img->height_/4;
		int realwidth = (img->width_ < 16) ? img->width_ : img->width_/4;
		
		const int numImages = img->getNumImages();
		const int numMipMaps = img->getNumMipMaps();

		const unsigned char* baseptr = img->getDataPtr();
		for (int image = 0; image < numImages; ++image)
		{
			for (int mipmap = 0; mipmap < numMipMaps; ++mipmap)
			{
				int currentWidth = realwidth >> mipmap; if (currentWidth < 16) currentWidth = 16;
				int currentHeight = realheight >> mipmap; if (currentHeight < 16) currentHeight = 16;
				
				const ptrdiff_t offset = img->getDataPtr(mipmap,image) - baseptr;	// get an offset in bytes to the correct image:mipmap data

				if(flips.flipBoth())
				{

					const int len = currentHeight * currentWidth;
					forward_mem_iter src(reinterpret_cast<T*>(data.get() + headeroffset + offset));
					forward_mem_iter end(reinterpret_cast<T*>(data.get() + headeroffset + offset) + len);
					reverse_mem_iter dest(reinterpret_cast<T*>(img->getDataPtr(mipmap,image)) + len - 1);

					std::transform(src,end,dest,boost::bind<T>(fullFlip, _1));

					//	std::transform(src,end,dest,fullFlip);

				}// TODO: redo the below two functions to take into account the fact we are working with blocks of 16 pixels at a time
				else if (flips.flipy())
				{
					for(int i = 0; i < currentHeight; ++i)
					{
						forward_mem_iter src(reinterpret_cast<T*>(data.get()+headeroffset + offset) + currentWidth*i);
						forward_mem_iter end(reinterpret_cast<T*>(data.get()+headeroffset + offset) + currentWidth*(i+1));

						const int destoffset = currentWidth * currentHeight - ((i+1) * currentWidth);
						forward_mem_iter dest(reinterpret_cast<T*>(img->getDataPtr(mipmap,image)) + destoffset);

						std::transform(src,end,dest,boost::bind<T>(yFlip, _1));
						//std::transform(src,end,dest,yFlip);
					}
				}
				else if(flips.flipx())
				{
					for (int i = 0; i < currentHeight; ++i)
					{
						forward_mem_iter src(reinterpret_cast<T*>(data.get()+headeroffset + offset) + currentWidth*i);
						forward_mem_iter end(reinterpret_cast<T*>(data.get()+headeroffset + offset) + currentWidth*(i+1));
						const int destoffset = /*currentWidth*currentHeight -*/ ((i+1)*currentWidth) - 1;
						reverse_mem_iter dest(reinterpret_cast<T*>(img->getDataPtr(mipmap,image)) + destoffset);

						std::transform(src,end,dest,boost::bind<T>(xFlip, _1));
						//std::transform(src,end,dest,xFlip);
					}
				}
				else
				{
					const int len = currentWidth * currentHeight;
					forward_mem_iter src(reinterpret_cast<T*>(data.get()+headeroffset + offset));
					forward_mem_iter end(reinterpret_cast<T*>(data.get()+headeroffset + offset) + len);
					forward_mem_iter dest(reinterpret_cast<T*>(img->getDataPtr(mipmap,image)));
					//std::transform(src,end,dest,func);
					std::copy(src,end,dest);
				}
			}
		}
		
	}

// TGA copying routines

	template<class T, class U>
	void PerformCopy(T src, T srcend, U dest)
	{
		std::copy(src,srcend, dest);
	}

	template<class T, class U>
	void PerformRepeatedCopy(T src, U dest, int count)
	{
		std::fill_n(dest,count,*src);
	}

	template<class U>
	void PerformCopy(GTLMemoryIterators::forward_memory_iter<GTLUtils::rgba> src, GTLMemoryIterators::forward_memory_iter<GTLUtils::rgba> srcend, U dest)
	{
		std::transform(src,srcend, dest, GTLUtils::convertBGRAtoRGBA);
	}

	template<class U>
	void PerformRepeatedCopy(GTLMemoryIterators::forward_memory_iter<GTLUtils::rgba> src, U dest, int count)
	{
		GTLUtils::rgba val = GTLUtils::convertBGRAtoRGBA(*src);
		std::fill_n(dest,count,val);
	}

	template<class T>
	void CopyData(T * src, T* dest, int width, int height, int count, int offset, GTLUtils::flipResult flips, bool repeated)
	{
		using namespace GTLMemoryIterators;
		typedef typename forward_memory_iter<T> forward_mem_iter;
		typedef typename reverse_memory_iter<T> reverse_mem_iter;

		if(flips.flipBoth())
		{
			const int imgsize = width * height;
			forward_mem_iter src_itor(src);
			forward_mem_iter end_itor(src + count);
			reverse_mem_iter dest_itor(dest + imgsize - offset - 1);
			if (repeated)
				PerformRepeatedCopy(src_itor,dest_itor,count);
			else
				PerformCopy(src_itor,end_itor,dest_itor);

		}
		else if (flips.flipy())
		{
			const int imgsize = width * height;
			while (count > 0)
			{
				int row = offset / width;	// get the row we are on i.e. 12 / 10 = row 1 (check interger divsion...)
				int rowRemains = width - (offset % width);
				int copyAmount = (count > rowRemains) ? rowRemains : count;

				forward_mem_iter src_itor(src);
				forward_mem_iter end_itor(src + copyAmount);
				forward_mem_iter dest_itor(dest + imgsize - ((row + 1) * width) + (width - rowRemains));
				if (repeated)
					PerformRepeatedCopy(src_itor,dest_itor,copyAmount);
				else
					PerformCopy(src_itor,end_itor,dest_itor);

				count -= copyAmount;
				offset += copyAmount;
				src += copyAmount;
			}
		} 
		else if(flips.flipx())
		{
//			const int imgsize = width * height;
			while (count > 0)
			{
				int row = offset / width;	// get the row we are on i.e. 12 / 10 = row 1 (check interger divsion...)
				int rowRemains = width - (offset % width);
				int copyAmount = (count > rowRemains) ? rowRemains : count;

				forward_mem_iter src_itor(src);
				forward_mem_iter end_itor(src + copyAmount);
				int rowoffset = ((row+1) * width) - (width - rowRemains) - 1;
				reverse_mem_iter dest_itor(dest + rowoffset );

				if (repeated)
					PerformRepeatedCopy(src_itor,dest_itor,copyAmount);
				else
					PerformCopy(src_itor,end_itor,dest_itor);

				count -= copyAmount;
				offset += copyAmount;
				src += copyAmount;
			}
		}
		else
		{
			forward_mem_iter src_itor(src);
			forward_mem_iter end_itor(src + count);
			forward_mem_iter dest_itor(dest + offset);
			if (repeated)
				PerformRepeatedCopy(src_itor,dest_itor,count);
			else
				PerformCopy(src_itor,end_itor,dest_itor);
		}
/*		if(flips.flipBoth())
		{
			const int imgsize = width * height;
			forward_mem_iter src_itor(src);
			forward_mem_iter end_itor(src + count);
			reverse_mem_iter dest_itor(dest + imgsize - offset);
			if (repeated)
				PerformRepeatedCopy(src_itor,dest_itor,count);
			else
				PerformNormalCopy(src_itor,end_itor,dest_itor);

		}
		else if (flips.flipy())
		{
			const int imgsize = width * height;
			while (count > 0)
			{
				int row = offset / width;	// get the row we are on i.e. 12 / 10 = row 1 (check interger divsion...)
				int rowRemains = width - (offset % width);
				int copyAmount = (count > rowRemains) ? rowRemains : count;
				
				forward_mem_iter src_itor(src);
				forward_mem_iter end_itor(src + copyAmount);
				forward_mem_iter dest_itor(dest + imgsize - (row * width));
				if (repeated)
					PerformRepeatedCopy(src_itor,dest_itor,copyAmount);
				else
					PerformNormalCopy(src_itor,end_itor,dest_itor);

				count -= copyAmount;
				offset += copyAmount;
			}
		}
		else if(flips.flipx())
		{

			const int imgsize = width * height;
			while (count > 0)
			{
				int row = offset / width;	// get the row we are on i.e. 12 / 10 = row 1 (check interger divsion...)
				int rowRemains = width - (offset % width);
				int copyAmount = (count > rowRemains) ? rowRemains : count;

				forward_mem_iter src_itor(src);
				forward_mem_iter end_itor(src + copyAmount);
				reverse_mem_iter dest_itor(dest + imgsize - (row * width) - (width - rowRemains));

				if (repeated)
					PerformRepeatedCopy(src_itor,dest_itor,copyAmount);
				else
					PerformNormalCopy(src_itor,end_itor,dest_itor);

				count -= copyAmount;
				offset += copyAmount;
			}
		}
		else
		{
			forward_mem_iter src_itor(src);
			forward_mem_iter end_itor(src + count);
			forward_mem_iter dest_itor(dest);
			if (repeated)
				PerformRepeatedCopy(src_itor,dest_itor,count);
			else
				PerformNormalCopy(src_itor,end_itor,dest_itor);
		}
		*/
	}
	
/*
	template<class T, class U>
	void PerformTransformedCopy(T src, T srcend, U dest)
	{
		std::transform(src,srcend, dest, GTLUtils::convertBGRAtoRGBA);
	}

	template<class T, class U>
	void PerformTransformedRepeatedCopy(T src, U dest, int count)
	{
		GTLUtils::rgba val = GTLUtils::convertBGRAtoRGBA(*src);
		std::fill_n(dest,count,val);
	}

	template<>
	void CopyData(GTLUtils::rgba * src, GTLUtils::rgba* dest, int width, int height, int count, int offset, GTLUtils::flipResult flips, bool repeated);
	*/
/*	{
		using namespace GTLMemoryIterators;
		typedef forward_memory_iter<GTLUtils::rgba> forward_mem_iter;
		typedef reverse_memory_iter<GTLUtils::rgba> reverse_mem_iter;

		if(flips.flipBoth())
		{
			const int imgsize = width * height;
			forward_mem_iter src_itor(src);
			forward_mem_iter end_itor(src + count);
			reverse_mem_iter dest_itor(dest + imgsize - offset);
			if (repeated)
				PerformTransformedRepeatedCopy(src_itor,dest_itor,count);
			else
				PerformTransformedCopy(src_itor,end_itor,dest_itor);

		}
		else if (flips.flipy())
		{
			const int imgsize = width * height;
			while (count > 0)
			{
				int row = offset / width;	// get the row we are on i.e. 12 / 10 = row 1 (check interger divsion...)
				int rowRemains = width - (offset % width);
				int copyAmount = (count > rowRemains) ? rowRemains : count;

				forward_mem_iter src_itor(src);
				forward_mem_iter end_itor(src + copyAmount);
				forward_mem_iter dest_itor(dest + imgsize - (row * width));
				if (repeated)
					PerformTransformedRepeatedCopy(src_itor,dest_itor,copyAmount);
				else
					PerformTransformedCopy(src_itor,end_itor,dest_itor);

				count -= copyAmount;
				offset += copyAmount;
			}
		}
		else if(flips.flipx())
		{

			const int imgsize = width * height;
			while (count > 0)
			{
				int row = offset / width;	// get the row we are on i.e. 12 / 10 = row 1 (check interger divsion...)
				int rowRemains = width - (offset % width);
				int copyAmount = (count > rowRemains) ? rowRemains : count;

				forward_mem_iter src_itor(src);
				forward_mem_iter end_itor(src + copyAmount);
				reverse_mem_iter dest_itor(dest + imgsize - (row * width) - (width - rowRemains));

				if (repeated)
					PerformTransformedRepeatedCopy(src_itor,dest_itor,copyAmount);
				else
					PerformTransformedCopy(src_itor,end_itor,dest_itor);

				count -= copyAmount;
				offset += copyAmount;
			}
		}
		else
		{
			forward_mem_iter src_itor(src);
			forward_mem_iter end_itor(src + count);
			forward_mem_iter dest_itor(dest);
			if (repeated)
				PerformTransformedRepeatedCopy(src_itor,dest_itor,count);
			else
				PerformTransformedCopy(src_itor,end_itor,dest_itor);
		}
	}
*/
	template<class T>
	bool decodeRLEData(GameTextureLoader3::ImagePtr image, 
		GameTextureLoader3::DecoderImageData data, GTLUtils::flipResult flips, int offset)
	{
		
		
		GTLCore::ImageImpl * img = dynamic_cast<GTLCore::ImageImpl*>(image.get());
		const int pixelsize = img->colourdepth_/8;
		const int size = img->width_ * img->height_ * pixelsize;
		const bool repeated = true;

		signed int currentbyte = 0;
		GTLUtils::byte * bytedata = reinterpret_cast<GTLUtils::byte*>(data.get() + offset);
		T * localimg = reinterpret_cast<T*>(img->imgdata_.get());

		while(currentbyte < size )
		{
			GTLUtils::byte chunkheader = *bytedata; bytedata++;
			// I'm pretty sure the line below here is wrong and shouldn't be there...
			//			++currentbyte;
			T * localdata = reinterpret_cast<T*>(bytedata);
			

			if(chunkheader < 128) // Then the next chunkheader++ blocks of data are unique pixels
			{
				chunkheader++;
				CopyData(localdata, localimg, img->width_, img->height_, chunkheader,(currentbyte/pixelsize),flips,!repeated);
				bytedata += chunkheader * pixelsize;
			}
			else	// RLE compressed data, chunkheader -127 gives us the total number of repeats
			{
				chunkheader -= 127;
				CopyData(localdata,localimg, img->width_, img->height_, chunkheader,(currentbyte/pixelsize),flips,repeated);
				bytedata+=pixelsize;	// move on the size of a pixel
			}
			currentbyte += pixelsize * chunkheader;
		}
		return true;
	}
}

#endif