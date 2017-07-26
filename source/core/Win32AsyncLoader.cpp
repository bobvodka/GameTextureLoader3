// Win32 based Async loading system.
#include "gtlcore.hpp"
#include <vector>
#include <queue>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace GTLCore
{
	namespace
	{
		std::queue<DWORD> threadQueue;

	}
#define WM_NEWWORK WM_USER+1
#define WM_QUITTHREAD WM_USER+2

	// Thread which does all the work
	DWORD WINAPI LoaderThread(LPVOID )
	{
		MSG msg;
		BOOL bRet;
		while ( (bRet = GetMessage(&msg, reinterpret_cast<HWND>(-1), WM_NEWWORK,WM_QUITTHREAD)) != 0)
		{
			if(msg.message == WM_NEWWORK)
			{
				WorkPacket *work = reinterpret_cast<WorkPacket*>(msg.lParam);
				LoadImageSync(work);
			}
			else if(msg.message == WM_QUITTHREAD)
			{
				break;
			}
//			delete work;
		}
		return 0;	
	}

	void ShutDownThreads()
	{
		while(threadQueue.size() > 0)
		{
			DWORD threadid = threadQueue.front();
			PostThreadMessage(threadid,WM_QUITTHREAD,0,0);
			HANDLE threadhnd = OpenThread(THREAD_QUERY_INFORMATION,false,threadid);
			DWORD rtn;
			GetExitCodeThread(threadhnd,&rtn);
			CloseHandle(threadhnd);
			threadQueue.pop();
		}
	}

	void ThreadInitalise(int workerThreads)
	{
		for (int i = 0; i < workerThreads; ++i)
		{
			DWORD threadid;
			CreateThread(NULL, 0,LoaderThread,NULL,NULL,&threadid );
			threadQueue.push(threadid);
		}
#ifndef GTL_BUILD_FOR_DLL
		atexit(ShutDownThreads);
#endif
	}

	void LoadImageAsync(WorkPacket *work)
	{
		if(threadQueue.empty())
			throw std::runtime_error("ThreadQueue: No Active Handles, Did you already shutdown the threading system?");

		DWORD threadid = threadQueue.front();
		threadQueue.pop();
		PostThreadMessage(threadid,WM_NEWWORK,0,reinterpret_cast<LPARAM>(work));
		threadQueue.push(threadid);
	}
}