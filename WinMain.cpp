/*
 * WinMain.cpp
 *
 *  This program is made by a hobby-ist to play with / learn C++ and Win32.
 *  Most of the things in the program are over-kill.
 *  But this is what happens when you want to learn something new - you end up wanting to try all sorts of things.
 *
 */


#include <windows.h>
#include "WinFrame.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{

	WinFrame *winFrame = new WinFrame();
	if(winFrame->CreateWindowClass(hInstance) != SUCCESS) return 0;
	if(winFrame->InitWinFrame() != SUCCESS) return 0;
	winFrame->ShowWinFrame();

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete winFrame;

	return 0;
}
