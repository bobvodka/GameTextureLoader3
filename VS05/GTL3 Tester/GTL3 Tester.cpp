// GTL3 Tester.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../includes/gtl/GameTextureLoader.hpp"
#pragma comment(lib,"gtl3.lib")

#include <vector>
#include <string>
#include <boost/assign/std/vector.hpp>

#include <iostream>
#include <fstream>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <boost/foreach.hpp>

#include <boost/lexical_cast.hpp>

#include <Windows.h>
#include <mmsystem.h>
#pragma comment(lib,"Winmm.lib")

struct testData
{
	testData(std::string filename, int width, int height, int colourdepth, GameTextureLoader3::ImgFormat format)
		: filename(filename), width(width), height(height), colourdepth(colourdepth), format(format)
	{

	}

	std::string filename;
	int width, height, colourdepth;
	GameTextureLoader3::ImgFormat format;
};

void savePNG(std::string filename, GameTextureLoader3::ImagePtr img);

boost::mutex lock;
volatile int loaded;

void Loaded(GameTextureLoader3::ImagePtr img, std::string const &filename)
{
	boost::mutex::scoped_lock scopedlock(lock);
	std::cout << "Filename : " << filename << std::endl;
	std::cout << boost::format("File details: \nWidth %1%\nHeight %2%\nFormat %3%\nColours %4%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth();
	loaded++;
}

void Error(GameTextureLoader3::ImagePtr img, std::string const &filename)
{
	boost::mutex::scoped_lock scopedlock(lock);
	std::cout << "Filename : " << filename << std::endl;
	std::cout << "Load error : " + img->getErrorInfo() << std::endl;
	loaded++;
}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace boost::assign; // bring 'operator+=()' into scope

	std::cout << "Setting up with 5 threads" << std::endl;
	GameTextureLoader3::Initalise(5);	// setup with 5 worker threads

	std::vector<testData> extensions;
	extensions += /*testData(".bmp",256,256,24,GameTextureLoader3::FORMAT_BGR)
		,testData(".tga",256,256,24,GameTextureLoader3::FORMAT_BGR)
		,testData("24RLE.tga",256,256,24,GameTextureLoader3::FORMAT_BGR)
		,testData("32.tga",256,256,32,GameTextureLoader3::FORMAT_RGBA)
		,testData("32RLE.tga",256,256,32,GameTextureLoader3::FORMAT_RGBA)
		,testData(".png",256,256,24,GameTextureLoader3::FORMAT_RGBA)
//		,testData("32.png",256,256,32,GameTextureLoader3::FORMAT_RGBA)
		,testData(".jpg",256,256,24,GameTextureLoader3::FORMAT_RGB)
		,testData(".dds",256,256,0,GameTextureLoader3::FORMAT_DXT1)
		,testData("3.dds",256,256,0,GameTextureLoader3::FORMAT_DXT3)
		,testData("5.dds",256,256,0,GameTextureLoader3::FORMAT_DXT5)
		// 1080i test data
		,testData("1080i.tga",256,256,24,GameTextureLoader3::FORMAT_BGR)
		,testData("1080iRLE.tga",256,256,24,GameTextureLoader3::FORMAT_BGR)
		,testData("1080i32.tga",256,256,32,GameTextureLoader3::FORMAT_RGBA)
		,testData("1080i32RLE.tga",256,256,32,GameTextureLoader3::FORMAT_RGBA)
*/		testData("32.png",256,256,32,GameTextureLoader3::FORMAT_RGBA)
		,testData("http://www.phantom-web.co.uk/wp-content/2007/12/snapshot20070814171603.jpg",256,256,32,GameTextureLoader3::FORMAT_RGB)
		,testData("http://www.phantom-web.co.uk/wp-content/2007/12/snapshot20070814171603.png",256,256,32,GameTextureLoader3::FORMAT_RGB);
		
	

	std::vector<GameTextureLoader3::ImgOrigin> origins;
	origins += GameTextureLoader3::ORIGIN_TOP_RIGHT
		/*,GameTextureLoader3::ORIGIN_BOTTOM_RIGHT
		,GameTextureLoader3::ORIGIN_BOTTOM_LEFT
		,GameTextureLoader3::ORIGIN_TOP_LEFT
		*/;

	std::ofstream outfile("results.csv");
	outfile << "filename,origin,width,height,format,colours,load time\n";

	timeBeginPeriod(1);

	std::cout << "Sync Loader Test" << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	BOOST_FOREACH(GameTextureLoader3::ImgOrigin origin, origins)
	{
		GameTextureLoader3::SetOrigin(origin);
		std::cout << boost::format("Origin value : %1%\n") % origin;
	//		outfile << boost::format("Origin value : %1%\n") % origin;
		BOOST_FOREACH(testData test, extensions)
		{
			std::string filename; 
			if(test.filename.find("http://") == 0
				|| test.filename.find("ftp://") == 0)
			{
				filename = test.filename;
			}
			else
			{
				filename = "test" + test.filename; 
			}
			std::cout << "Loading : " << filename << std::endl;
	//			outfile << "Loading : " << filename << std::endl;
			DWORD start = timeGetTime();
			GameTextureLoader3::ImagePtr img = GameTextureLoader3::LoadTexture(filename);
			DWORD end = timeGetTime();
			if(GameTextureLoader3::COMPLETED == img->getState())
			{
				std::cout << boost::format("File details: \nWidth %1%\nHeight %2%\nFormat %3%\nColours %4%\nLoad Time (ms) %5%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth() % (end - start);
	//			outfile << boost::format("File details: \nWidth %1%\nHeight %2%\nFormat %3%\nColours %4%\nLoad Time (ms) %5%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth() % (end - start);
				outfile << boost::format("%7%,%6%,%1%,%2%,%3%,%4%,%5%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth() % (end - start) % origin % filename;
				std::string outfilename = filename + "." + boost::lexical_cast<std::string>(origin) + ".png";
				savePNG(outfilename,img);
			}
			else
			{
				std::cout << "Load error : " + img->getErrorInfo() << std::endl;
			}
			
		}
		std::cout << "------------------------------------------------------------" << std::endl;
	//		outfile << "------------------------------------------------------------" << std::endl;
		outfile << std::endl;
	}

	std::cout << "Async Loader Test" << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	BOOST_FOREACH(GameTextureLoader3::ImgOrigin origin, origins)
	{
		GameTextureLoader3::SetOrigin(origin);
		std::cout << boost::format("Origin value : %1%\n") % origin;
		//		outfile << boost::format("Origin value : %1%\n") % origin;
		BOOST_FOREACH(testData test, extensions)
		{
			std::string filename; 
			if(test.filename.find("http://") == 0
				|| test.filename.find("ftp://") == 0)
			{
				filename = test.filename;
			}
			else
			{
				filename = "test" + test.filename; 
			}

			std::cout << "Loading : " << filename << std::endl;
			//			outfile << "Loading : " << filename << std::endl;
			DWORD start = timeGetTime();
			GameTextureLoader3::ImagePtr img = GameTextureLoader3::LoadTexture(filename,GameTextureLoader3::ASYNC);
			std::cout << "... Processing Async" << std::endl;
			while(img->getState() == GameTextureLoader3::PENDING)
			{
				boost::thread::yield();
			}
			std::cout << "... Loading" << std::endl;
			while (img->getState() == GameTextureLoader3::LOADING)
			{
				boost::thread::yield();
			}
			while (img->getState() == GameTextureLoader3::LOADED)
			{
				boost::thread::yield();
			}
			std::cout << "... Loaded" << std::endl;
			std::cout << "... Processing...";
			while(img->getState() == GameTextureLoader3::PROCESSING)
			{
				boost::thread::yield();
			}
			std::cout << " Completed" << std::endl;
			if(GameTextureLoader3::COMPLETED == img->getState())
			{
				std::cout << "... Completed" << std::endl;
				DWORD end = timeGetTime();
				std::cout << boost::format("File details: \nWidth %1%\nHeight %2%\nFormat %3%\nColours %4%\nLoad Time (ms) %5%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth() % (end - start);
				//			outfile << boost::format("File details: \nWidth %1%\nHeight %2%\nFormat %3%\nColours %4%\nLoad Time (ms) %5%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth() % (end - start);
				outfile << boost::format("%7%,%6%,%1%,%2%,%3%,%4%,%5%\n") % img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth() % (end - start) % origin % filename;
			}
			else
			{
				std::cout << "Load error : " + img->getErrorInfo() << std::endl;
			}
		}
		std::cout << "------------------------------------------------------------" << std::endl;
		//		outfile << "------------------------------------------------------------" << std::endl;
		outfile << std::endl;
	}

	std::cout << "Callback Loader Test" << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	GameTextureLoader3::SetCallBacks(Loaded,Error);

	BOOST_FOREACH(GameTextureLoader3::ImgOrigin origin, origins)
	{
		GameTextureLoader3::SetOrigin(origin);
		std::cout << boost::format("Origin value : %1%\n") % origin;
		BOOST_FOREACH(testData test, extensions)
		{
			std::string filename; 
			if(test.filename.find("http://") == 0
				|| test.filename.find("ftp://") == 0)
			{
				filename = test.filename;
			}
			else
			{
				filename = "test" + test.filename; 
			}
			GameTextureLoader3::ImagePtr img = GameTextureLoader3::LoadTexture(filename);
		}
		std::cout << "------------------------------------------------------------" << std::endl;
	}

	std::cout << "Async Callback Loader Test" << std::endl;
	std::cout << "------------------------------------------------------------" << std::endl;

	GameTextureLoader3::SetCallBacks(Loaded,Error);

	std::vector<testData> extensions2;
	extensions2 += testData("http://www.phantom-web.co.uk/wp-content/2007/12/snapshot20070814171603.jpg",256,256,32,GameTextureLoader3::FORMAT_RGB)
				  ,testData(".bmp",256,256,24,GameTextureLoader3::FORMAT_BGR)
				  ,testData(".tga",256,256,24,GameTextureLoader3::FORMAT_BGR)
				  ,testData("24RLE.tga",256,256,24,GameTextureLoader3::FORMAT_BGR)
				  ,testData("32.tga",256,256,32,GameTextureLoader3::FORMAT_RGBA)
				  ,testData("32RLE.tga",256,256,32,GameTextureLoader3::FORMAT_RGBA)
				  ,testData(".png",256,256,24,GameTextureLoader3::FORMAT_RGBA)
				  ,testData(".jpg",256,256,24,GameTextureLoader3::FORMAT_RGB)
				  ,testData(".dds",256,256,0,GameTextureLoader3::FORMAT_DXT1)
				  ,testData("3.dds",256,256,0,GameTextureLoader3::FORMAT_DXT3)
				  ,testData("5.dds",256,256,0,GameTextureLoader3::FORMAT_DXT5)
				;

	
	GameTextureLoader3::SetOrigin(GameTextureLoader3::ORIGIN_TOP_LEFT);
	loaded = 0;
	BOOST_FOREACH(testData test, extensions2)
	{
		std::string filename; 
		if(test.filename.find("http://") == 0
			|| test.filename.find("ftp://") == 0)
		{
			filename = test.filename;
		}
		else
		{
			filename = "test" + test.filename; 
		}
		GameTextureLoader3::ImagePtr img = GameTextureLoader3::LoadTexture(filename,GameTextureLoader3::ASYNC);
	}

	while(loaded < 11)
		boost::thread::yield();

	std::cout << "------------------------------------------------------------" << std::endl;
	

	timeEndPeriod(1);

	

	std::cin.get();
	
	return 0;
}

/*
std::cout << "Initalising with 5 threads" << std::endl;
GameTextureLoader3::Initalise(5);	// setup with 5 worker threads

std::cout << "Loading test.bmp" << std::endl;
GameTextureLoader3::ImagePtr img = GameTextureLoader3::LoadTexture("test.bmp");

std::cout << boost::format("Width %1%\nHeight %2%\nFormat %3%\nColours %4%\n")
% img->getWidth() % img->getHeight() % img->getFormat() % img->getColourDepth();

GameTextureLoader3::ImagePtr img2 = GameTextureLoader3::LoadTexture("test.bmp",GameTextureLoader3::ASYNC);
while(img2->getState() == GameTextureLoader3::PENDING)
{
boost::thread::yield();
}
std::cout << "Processing Async" << std::endl;
while(img2->getState() == GameTextureLoader3::PROCESSING)
{
boost::thread::yield();
}
if (img2->getState() == GameTextureLoader3::ERROR)
{
std::cout << "Error loading..." << std::endl;
}
else
{
std::cout << boost::format("Width %1%\nHeight %2%\nFormat %3%\nColours %4%\n")
% img2->getWidth() % img2->getHeight() % img2->getFormat() % img2->getColourDepth();
}	

std::cin.get();

return 0;
*/
