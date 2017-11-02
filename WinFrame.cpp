/*
 * WinFrame.cpp
 *
 */

#include "WinFrame.h"

const wchar_t WinFrame::CLASS_NAME[]  = L"Stock Ticker Wnd Class";

WinFrame::WinFrame() :
		_tickerTape(),
		_tickerEB(),
		_workerThread()
{
	 _hWnd = NULL;
	 _hInstance = NULL;
	 _offset = 0;

	 _trackingMouse = FALSE;

	 // assume the ticker edit box will be visible from the start. Maybe change this later.
	 _tickerEBVisible = TRUE;
	 _tickerEBHFont = NULL;
	 _tickerEBHeight = 0;
	 memset(&_wc,0, sizeof(WNDCLASSEX));
}

//
// 	Creates and registers the window class for the frame.
//
FUNC_RESULT WinFrame::CreateWindowClass(HINSTANCE hInstance)
{

	_hInstance = hInstance;

	// Register the window class
	_wc.cbSize = sizeof(_wc);
	_wc.cbClsExtra = 0;
	_wc.cbWndExtra = 0;
	_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	_wc.style = CS_HREDRAW | CS_VREDRAW;
	_wc.lpfnWndProc   = WinFrame::WindowProc;
	_wc.hInstance     = hInstance;
	_wc.lpszClassName = WinFrame::CLASS_NAME;
	_wc.hbrBackground = CreateSolidBrush(StockTickerBackgroundColor);

	ATOM res = RegisterClassEx(&_wc);

	if (!res) return ERR_WINDOW_NOT_REGISTERED;

	return SUCCESS;
}

//
// 	Creates the window.
//	Creates the font to draw
//
FUNC_RESULT WinFrame::InitWinFrame()
{

	// Create the window
	 _hWnd = CreateWindowEx
			 (
			        0,                              // Optional window styles.
			        WinFrame::CLASS_NAME,			// Window class
			        NULL, 							// Window text
			        WS_SIZEBOX | WS_POPUP,			// Window style. WS_POPUP so the windows does not have caption, WS_SIZEBOX so the window is re-sizable.

			        // Size and position

			        CW_USEDEFAULT,					// x
			        CW_USEDEFAULT, 					// y
			        CW_USEDEFAULT, 					// nWidth
			        CW_USEDEFAULT,					// nHeight

			        NULL,       					// Parent window
			        NULL,       					// Menu
			        _hInstance,  					// Instance handle
			        this        					// Additional application data
			 );

	 if (!_hWnd) return ERR_WINDOW_NOT_CREATED;

	 SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR) this);

	 // Set the timers for the ticker tape and for worker thread
	 SetTimer(_hWnd, WinFrame::tickerTapeTimer_ID, WinFrame::tickerTapeTimer_Int, NULL);
	 SetTimer(_hWnd, WinFrame::workerThreadTimer_ID, WinFrame::workerThreadTimer_Int, NULL);
	 SetTimer(_hWnd, WinFrame::workerThreadPollTimer_ID, WinFrame::workerThreadPollTimer_Int, NULL);

	 // calculate the horiz / vert sizing border. Will need later for positioning the ticker edit box
	_horizSizingBorderThicknes = GetSystemMetrics(SM_CXFRAME);
	_vertSizingBorderThicknes = GetSystemMetrics(SM_CYFRAME);


	 FUNC_RESULT result = InitializeTickerEditBox();
	 if (result != SUCCESS) return result;

	 // initialize the ticker tape.
	 result = _tickerTape.Init();
	 if (result != SUCCESS) return result;

	 // WorkerThread stuff
	 _uiState = DoesNotNeedData;
	 // Initialize WorkerThread and we are done.
	 return _workerThread.Init();
}

//
// handles the notification from the ticker edit box that
// the user finished inputting the stock quotes.
//
void WinFrame::TickerEditBoxUpdatedText(LPCWSTR tickerEditBoxText)
{
	if (_workerThread.UIThreadWorkerThreadBlocked())
	{
		_workerThread.UIThreadRequestsStockQuotes(tickerEditBoxText);
		_uiState = RequestedDataWaitingForData;
	}
	else
	{
		_uiState = NeedDataDidNotRequestDataYet;
	}

	int a = min (wcslen(tickerEditBoxText), WinFrame::symbolsBuffLen - 1);
	wcsncpy(_symbolsBuff/*dest*/, tickerEditBoxText/*src*/, a/*num*/);
	_symbolsBuff[a] = L'\0';


	_tickerTape.UpdateText(tickerEditBoxText);
	_offset = 0;
	RECT r; r.right = 0; r.top = 0; r.bottom = _clientHeight; r.left = _clientWidth;
	InvalidateRect(_hWnd, &r, TRUE);
}


//
// This function shows the window frame for the first time
//
void WinFrame::ShowWinFrame()
{
	ShowWindow(_hWnd, SW_SHOW /*nCmdShow*/);
	_tickerEB.ShowTickerEditBox();
}

WinFrame::~WinFrame()
{
	// Nothing to do. By now our resources should already have been cleaned up.
}

//
// Invoked on the WM_DESTROY message.
// Cleans up all the resources except _hwnd and _wc
void WinFrame::Cleanup()
{
	_tickerTape.Cleanup();
	KillTimer(_hWnd, WinFrame::tickerTapeTimer_Int);
	KillTimer(_hWnd, WinFrame::workerThreadTimer_ID);
	KillTimer(_hWnd, WinFrame::workerThreadPollTimer_ID);
	_hWnd = NULL;
	_hInstance = NULL;

	if (_tickerEBHFont) DeleteObject (_tickerEBHFont);
}

//
// Creates the font that the Ticked Edit Box will use to paint.
// The WinFrame "owns" this font; the WinFrame is responsible for deleting the font.
//
FUNC_RESULT WinFrame::CreateTickerEBFont()
{
	// Create the font
	 _tickerEBHFont = CreateFont
			 	 (
	             		20,						// Height
	             		0,						// Width - go with Windows default
	             		0,						// Escapement
	             		0,						// Orientation
	             		FW_DONTCARE,			// Weight
	             		FALSE,		 			// Italic
	             		FALSE,					// Underline
	             		FALSE,					// StrikeOut
	             		DEFAULT_CHARSET,		// CharSet
	             		OUT_TT_ONLY_PRECIS,		// OutputPrecision - pick up a TrueType font
	             		CLIP_DEFAULT_PRECIS,	// ClipPrecision
	             		ANTIALIASED_QUALITY,	// Quality
	             		DEFAULT_PITCH,			// Pitch and Family
	             		L"Arial"
			 	 );

	 if (!_tickerEBHFont) return ERR_TICKERBAND_FONT_NOT_CREATED;
	return SUCCESS;
}

//
// Initializes the Ticker Edit Box.
// It creates the window class for the TickerEB,
// creates the font TickerEB will use
// calculates the position of the TickerEB
//
FUNC_RESULT WinFrame::InitializeTickerEditBox()
{
	FUNC_RESULT result;

	result = _tickerEB.CreateWindowClass(_hInstance);
	if (result != SUCCESS) return result;

	result = CreateTickerEBFont();
	if (result != SUCCESS) return result;

	// calculate the height of the TickerEditBox
	//
	TEXTMETRIC tm;
	HDC hdc = GetDC(_hWnd);
	SelectObject(hdc, _tickerEBHFont);
	if (!GetTextMetrics(hdc, &tm)) return ERR_GET_TEXT_METRICS_FAIL;
	_tickerEBHeight = tm.tmHeight;
	ReleaseDC(_hWnd, hdc);

	result = _tickerEB.InitTickerEditBox(
							this,															// pointer to us.
							_hWnd, 															// winFrameHWnd
							_tickerEBHFont,													// winFrameTickerEBFont
							_horizSizingBorderThicknes,										// x
							_clientHeight - _tickerEBHeight + _vertSizingBorderThicknes,	// y
							_clientWidth - WinFrame::tickerPadding, 						// nWidth
							_tickerEBHeight	 - WinFrame::tickerPadding						// nHeight
							);

	return result;
}

// The Window Proc for WinFrame.
LRESULT CALLBACK WinFrame::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
		{
			// Good bye, cruel world...
			WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
			winFrame->Cleanup();

			// Wait for the WorkerThread to exit
			winFrame->_workerThread.UIThreadRequestsEndThread();

			PostQuitMessage(0);
			return 0;
		}
    case WM_CREATE:
		{
			CREATESTRUCT * cs = reinterpret_cast <CREATESTRUCT *> (lParam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) cs->lpCreateParams);
			return 0;
		}
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint (hwnd, &ps);

            WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
            winFrame->_tickerTape.PaintTickerTape(winFrame->_offset, hdc, winFrame->_clientWidth, winFrame->_clientHeight);

            EndPaint(hwnd, &ps);
            return 0;
        }
    case WM_GETMINMAXINFO:
    	{
    		MINMAXINFO *minMaxInfo = reinterpret_cast <MINMAXINFO *> (lParam);
    		minMaxInfo->ptMaxSize.x = WinFrame::maxWidth;
    		minMaxInfo->ptMaxSize.y = WinFrame::maxHeight;
    		minMaxInfo->ptMaxTrackSize.x = WinFrame::maxWidth;
    		minMaxInfo->ptMaxTrackSize.y = WinFrame::maxHeight;
    		minMaxInfo->ptMinTrackSize.x = WinFrame::minWidth;
    		minMaxInfo->ptMinTrackSize.y = WinFrame::minHeight;

    		return 0;
    	}
    case WM_LBUTTONDOWN:
		{
            WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
            winFrame->_trackingMouse = TRUE;
            GetWindowRect(winFrame->_hWnd, &(winFrame->_windowRect));
            GetCursorPos(&(winFrame->_cursorPos));
            return 0;
		}
    case WM_LBUTTONUP:
		{
            WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
            winFrame->_trackingMouse = FALSE;
            return 0;
		}
    case WM_MOUSEMOVE:
		{
            WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
            if (winFrame->_trackingMouse)
            {
            	POINT p;
            	GetCursorPos(&p);
            	int x = winFrame->_windowRect.left + p.x - winFrame->_cursorPos.x;
            	int y = winFrame->_windowRect.top + p.y - winFrame->_cursorPos.y;
            	SetWindowPos(
            				 winFrame->_hWnd,								// hWnd
            				 NULL,											// hWndInsertAfter
            				 x,												// X
            				 y,												// Y
            				 0,												// cx
            				 0,												// cy
            				 SWP_NOOWNERZORDER | SWP_NOREDRAW | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE // uiFlags
            				 );
                // reset the position for the Ticker Edit Box
                winFrame->_tickerEB.SetPosition
                							(
                									x + winFrame->_horizSizingBorderThicknes, 								// x
                									y + winFrame->_clientHeight - winFrame->_tickerEBHeight + winFrame->_vertSizingBorderThicknes,				// y
                									winFrame->_clientWidth - WinFrame::tickerPadding,					// nWidth
                									winFrame->_tickerEBHeight  - WinFrame::tickerPadding				// nHeight
                							);
            }
            return 0;
		}
    case WM_SIZE:
		{
            WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
            winFrame->_clientWidth = LOWORD (lParam);
            winFrame->_clientHeight = HIWORD (lParam);

            // reset the position for the Ticker Edit Box
            winFrame->_tickerEB.SetPosition
            							(
            									winFrame->_horizSizingBorderThicknes, 								// x
            									winFrame->_clientHeight - winFrame->_tickerEBHeight + winFrame->_vertSizingBorderThicknes,				// y
            									winFrame->_clientWidth - WinFrame::tickerPadding,					// nWidth
            									winFrame->_tickerEBHeight  - WinFrame::tickerPadding				// nHeight
            							);
            return 0;
		}
    case WM_TIMER:
		{
            WinFrame * winFrame = reinterpret_cast <WinFrame *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));

            if (wParam == WinFrame::tickerTapeTimer_ID)
            {
				// This is the TickerTapeTimer.
            	// Advance the offset
				winFrame->_offset += WinFrame::advanceOffset;
				int cycleWidth = max (winFrame->_clientWidth, winFrame->_tickerTape.GetTotalWidth());
				if (winFrame->_offset >= cycleWidth) winFrame->_offset -= cycleWidth;

				// Paint
				HDC hdc = GetDC(hwnd);
				winFrame->_tickerTape.PaintTickerTape(winFrame->_offset, hdc, winFrame->_clientWidth, winFrame->_clientHeight);
				ReleaseDC(hwnd, hdc);
            }
            else if (wParam == WinFrame::workerThreadTimer_ID)
            {
            	// This is the worker thread timer.
            	winFrame->_uiState = NeedDataDidNotRequestDataYet;
            }
            else if (wParam == WinFrame::workerThreadPollTimer_ID)
            {
            	// this is the timer we are polling on
            	if (winFrame->_uiState == NeedDataDidNotRequestDataYet)
            	{
            		if (winFrame->_workerThread.UIThreadWorkerThreadBlocked())
            		{
            			winFrame->_workerThread.UIThreadRequestsStockQuotes(winFrame->_symbolsBuff);
            			winFrame->_uiState = RequestedDataWaitingForData;
            		}
            	}
            	else if (winFrame->_uiState == RequestedDataWaitingForData)
            	{
            		if (winFrame->_workerThread.UIThreadWorkerThreadBlocked())
            		{
            			winFrame->_workerThread.UIThreadUpdateWithStockQuotes(winFrame->_quotesBuff, quotesBuffLen);
            			winFrame->_tickerTape.UpdateText(winFrame->_quotesBuff);

            			winFrame->_offset = 0;
            			RECT r; r.right = 0; r.top = 0; r.bottom = winFrame->_clientHeight; r.left = winFrame->_clientWidth;
            			InvalidateRect(winFrame->_hWnd, &r, TRUE);

            			winFrame->_uiState = DoesNotNeedData;
            		}
            	}
            }

            return 0;
		}
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
