/*
 * WinFrame.h
 *
 * Header file for the main window in the program.
 *
 */

#ifndef WINFRAME_H_
#define WINFRAME_H_

#include "Ticker.h"
#include "TickerTape.h"
#include "TickerEditBox.h"
#include "WorkerThread.h"

class WinFrame
{
public:
	WinFrame();
	FUNC_RESULT CreateWindowClass(HINSTANCE hInstance);
	FUNC_RESULT InitWinFrame();
	void TickerEditBoxUpdatedText(LPCWSTR tickerEditBoxText);
	void ShowWinFrame();
	virtual ~WinFrame();


private:

	// Private Enum
	//
	enum UIThreadState
	{
		DoesNotNeedData, 						// either WinFrame does not need data, or the WinFrame
		NeedDataDidNotRequestDataYet,			// we want stock quotes but did not get a chance to request data (because the worker thread was not blocked).
		RequestedDataWaitingForData, 			// want stock quotes and sent the request to the worker thread.
	};

	// Private constants
	//

	// size constants
	static const int maxWidth = 500;
	static const int maxHeight = 200;
	static const int minWidth = 200;
	static const int minHeight = 100;

	static const wchar_t CLASS_NAME[];

	// tickerTape constants
	//
	static const unsigned int tickerTapeTimer_ID = 1;
	static const int tickerTapeTimer_Int = 150;
	static const int advanceOffset = 3;

	// ticker edit box padding constants
	static const int tickerPadding = 2;

	// constants related to working with Worker Thread
	static const int quotesBuffLen = 1024;
	static const int symbolsBuffLen = 1024;
	static const unsigned int workerThreadTimer_ID = 2;
	static const int workerThreadTimer_Int = 1000 * 60 * 5;		// pull data from the internet every 5 minutes.
	static const unsigned int workerThreadPollTimer_ID = 3;		// WinFrame will poll the worker thread every 1 second
	static const int workerThreadPollTimer_Int = 1000;


	// Member Variables.
	//

	// WinFrame Window variables
	HINSTANCE _hInstance;
	WNDCLASSEX _wc;
	HWND _hWnd;

	int _clientWidth;
	int _clientHeight;

	int _horizSizingBorderThicknes;
	int _vertSizingBorderThicknes;

	// Variables for tracking the mouse (so the user can move the window)
	BOOL _trackingMouse;
	POINT _cursorPos;
	RECT _windowRect;

	// WinFrame Ticker Tape Variables
	TickerTape _tickerTape;
	int _offset;

	// WinFrame Ticker Edit Box Variables
	TickerEditBox _tickerEB;
	BOOL _tickerEBVisible;
	HFONT _tickerEBHFont;			// WinFrame owns this font. WinFrame creates and destroys this font.
	int _tickerEBHeight;


	// Member variables for working with the Worker Thread
	//
	// RULE OF THUMB: the WinFrame (a.k.a UI or UI Thread) will send stock symbols / retrieve stock quotes to / from
	//				  Worker Thread only when the WorkerThread is blocked waiting for work.
	//				  If WinFrame wants stock quotes but the worker thread was busy then WinFrame will be in NeedDataDidNotRequestDataYet.
	//				  In this state, WinFrame will periodically check if the worker thread is blocked.
	//		          When the worker thread is blocked, WinFrame will request stock quotes and set its state to RequestedDataWaitingForData
	//				  In this state, WinFrame will periodically check when worker thread is blocked again.
	//				  Once worker thread is blocked again, WinFrame will get its data and set its state to DidNotAskForData.
	//
	wchar_t _quotesBuff[quotesBuffLen];					// this buffer will hold the results from the WorkerThread
	wchar_t _symbolsBuff[symbolsBuffLen];				// this buffer will hold the symbols while waiting for the worker thread to block.
	UIThreadState _uiState;
	WorkerThread _workerThread;

	// The WindowProc for the Window Frame. Needs to be static
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	// Private functions
	void Cleanup();
	FUNC_RESULT CreateTickerEBFont();
	FUNC_RESULT InitializeTickerEditBox();

};

#endif /* WINFRAME_H_ */
