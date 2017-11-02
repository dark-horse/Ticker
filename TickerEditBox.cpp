/*
 * TickerEditBox.cpp
 *
 */

#include "TickerEditBox.h"
#include "WinFrame.h"
#include <stdio.h>

const wchar_t TickerEditBox::CLASS_NAME[]  = L"Stock Ticker Edit Box Window Class";

TickerEditBox::TickerEditBox()
{
	// Be bold, my friend. Zero everything out.
	memset (this, 0, sizeof(TickerEditBox));
}

//
// 	Creates and registers the window class for the frame.
//
FUNC_RESULT TickerEditBox::CreateWindowClass(HINSTANCE hInstance)
{

	_hInstance = hInstance;

	// Register the window class
	_wc.cbSize = sizeof(_wc);
	_wc.cbClsExtra = 0;
	_wc.cbWndExtra = 0;
	_wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
	_wc.style = CS_HREDRAW | CS_VREDRAW;
	_wc.lpfnWndProc   = TickerEditBox::WindowProc;
	_wc.hInstance     = hInstance;
	_wc.lpszClassName = TickerEditBox::CLASS_NAME;
	_wc.hbrBackground = CreateSolidBrush(TickerEditBox::BKCOLOR);

	if (!RegisterClassEx(&_wc)) return ERR_WINDOW_NOT_REGISTERED;

	return SUCCESS;
}

//
// This method will initialize the Window Class for the Stock Ticker Edit Box.
// This method will not show the Edit Box
//
FUNC_RESULT TickerEditBox::InitTickerEditBox(WinFrame *parentFrame, HWND winFrameHWnd, HFONT winFrameTickerEBFont, int x, int y, int nWidth, int nHeight)
{

	// Create the window
	_hWnd = CreateWindowEx
		 (
		        0,                              	// Optional window styles.
		        TickerEditBox::CLASS_NAME,			// Window class
		        NULL, 								// Window text
		        WS_POPUP | WS_CHILD | WS_BORDER,	// Window style.
		        									// WS_POPUP so the windows does not have caption.
		        									// WS_CHILD because it is a child window.
		        									// WS_BORDER so that the system paints the frame
		        									// Maybe add WS_CLIPCHILDREN | WS_CLIPSIBLINGS? so no other windows paint on top of us?

		        // Size and position

		        x,								// x
		        y,			 					// y
		        nWidth,		 					// nWidth
		        nHeight,						// nHeight

		        winFrameHWnd, 					// Parent window
		        NULL,       					// Menu
		        _hInstance,  					// Instance handle
		        this        					// Additional application data
		 );

	if (!_hWnd) return ERR_WINDOW_NOT_CREATED;

	_hFont = winFrameTickerEBFont;
	_parentFrame = parentFrame;
	_winFrameHWnd = winFrameHWnd;

	// init our ABC Widths array
	InitCharacterWidthsCache();


	SetWindowLongPtr(_hWnd, GWLP_USERDATA, (LONG_PTR) this);

	return SUCCESS;
}

//
// This method will show the Stock Ticker Edit Box.
// Will also activate the Stock Ticker Edit Box.
//
void TickerEditBox::ShowTickerEditBox()
{
	ShowWindow
			  (
					  _hWnd,
					  SW_SHOW 					// SW_SHOW - ACTIVATES the window (and shows it in its current size and position).
			  );
}

void TickerEditBox::SetPosition(int x, int y, int nWidth, int nHeight)
{
	SetWindowPos(_hWnd, NULL, x, y, nWidth, nHeight, 0);
}

//
// This method will hide the Stock Ticker Edit Box.
//
FUNC_RESULT TickerEditBox::HideTickerEditBox()
{
	return SUCCESS;
}

void TickerEditBox::TickerEditBoxRefreshed(LPCWSTR tickerEditBoxText)
{
	_parentFrame->TickerEditBoxUpdatedText(_txtBuff);
}

//
// Our place to cleanup our resources.
//
void TickerEditBox::Cleanup()
{
}

//
// Initialize our cache of character widths.
// Assumes we already have an _hWnd and a font (from winFrame)
//
void TickerEditBox::InitCharacterWidthsCache()
{
	HDC hdc = GetDC(_hWnd);
	SelectObject(hdc, _hFont);
	GetCharABCWidths(hdc, (UINT) TickerEditBox::firstChar, (UINT) TickerEditBox::lastChar, _ABCWidths);

	ReleaseDC(_hWnd, hdc);
}

//
// Calculates the width of inputChar when painted on the screen.
//
int TickerEditBox::CalcCharWidth(wchar_t inputChar)
{
	if (inputChar > TickerEditBox::lastChar)
	{
		return 0;
	}

	ABC abc = _ABCWidths[(UINT) inputChar - (UINT) TickerEditBox::firstChar];
	return abc.abcA +abc.abcB + abc.abcC;
}


//
// Reset the text box state.
//
void TickerEditBox::ResetTextBoxState()
{

	_currBuffOff = 0;
	_currXOffPix = 0;
	_firstVisChar = 0;
	_firstVisCharHiddenPix = 0;
	_lastPartVisChar = 0;
	_lastPartVisCharPix = 0;
	_txtPix = 0;

	if (_txtBuffLen == 0)
	{
		return;
	}

	while (_txtPix < _wndWidth && _lastPartVisChar < _txtBuffLen)
	{
		_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
		_txtPix += _lastPartVisCharPix;
		_lastPartVisChar ++;
	}

	if (_txtPix > _wndWidth)
	{
		_lastPartVisCharPix -= _txtPix - _wndWidth;
		_txtPix = _wndWidth;
	}
}


//
// For debugging purposes.
// Prints the state of the Ticker Edit Box to the standard output.
// Works only if linked with -mconsole option.
//
void TickerEditBox::PrintState()
{
	wprintf(L"State of the TickerEditBox:\r\n");


	wprintf(L"\t_txtBuff: %s\r\n",_txtBuff);
	wprintf(L"\t_txtBuffLen: \t\t\t\t%d\r\n",_txtBuffLen);
	wprintf(L"\t_currBuffOff: \t\t\t\t%d - %c\r\n",_currBuffOff, _txtBuff[_currBuffOff-1]);
	wprintf(L"\t_currXOffPix: \t\t\t\t%d\r\n",_currXOffPix);
	wprintf(L"\t_firstVisChar: \t\t\t\t%d - %c\r\n",_firstVisChar, _txtBuff[_firstVisChar]);
	wprintf(L"\t_firstVisCharHiddenPix: \t\t%d\r\n",_firstVisCharHiddenPix);
	wprintf(L"\t_lastPartVisChar: \t\t\t%d - %c\r\n",_lastPartVisChar, _txtBuff[_lastPartVisChar]);
	wprintf(L"\t_lastPartVisCharPix: \t\t\t%d\r\n",_lastPartVisCharPix);
	wprintf(L"\t_txtPix: \t\t\t\t%d\r\n",_txtPix);
	wprintf(L"\t_wndWidth: \t\t\t\t%d\r\n",_wndWidth);

	wprintf(L"\r\n\r\n\r\n");
}

//
// Scrolls xDelta pixels in the text box to the left starting at xSrc.
// Updates _txtPix, _lastVisChar, _lastVisCharPix.
// Handles the painting required for this operation.
//
void TickerEditBox::ScrollLeftWhenDelete(int xDest, int xSrc, int xDelta)
{
	// 1. Hide the caret
	// 2. Get the DC from the hwnd
	// 3. Update State & Paint (not necessarily in this order)
	// 4. Set Caret position and show the caret
	// 5. Release the DC
	// 6. Done

	// 1. Hide the caret
	HideCaret(_hWnd);

	// 2. Get the DC from the hwnd
	HDC hdc = GetDC(_hWnd);
	// 3. Update State & Paint (not necessarily in this order)

	BitBlt(
		hdc,											// destination HDC is the same as src HDC
		xDest,											// xDest
		0,												// yDest
		max(0,_wndWidth - xDelta),						// nWidth
		_wndHeight,										// nHeight
		hdc,											// source HDC
		xSrc,											// xSrc
		0,												// ySrc
		SRCCOPY											// Raster OP
		);

	// There are a few cases:
	// CASE 1 - the last character in the buffer was fully visible in the Ticker Edit Box.
	// CASE 2 - the last character in the buffer was only partially visible in the Ticker Edit Box.
	// CASE 3 - the last character in the buffer was not visible at all in the Ticker Edit Box.

	int lastCharWidth = CalcCharWidth(_txtBuff[_lastPartVisChar]);
	if (_lastPartVisChar == _txtBuffLen - 1 && _lastPartVisCharPix == lastCharWidth)
	{
		// CASE 1 - the last character in the buffer was fully visible in the Ticker Edit Box.
		// we only need to update _txtPix and fill rect.
		_txtPix -= xDelta;

		RECT r; r.left = _txtPix; r.top = 0; r.right = _wndWidth;r.bottom = _wndHeight;
		HBRUSH br = CreateSolidBrush(TickerEditBox::BKCOLOR);
		FillRect (hdc, &r, br);
		DeleteObject(br);
	}
	else if (_lastPartVisChar == _txtBuffLen - 1)
	{
		// CASE 2 - the last character in the buffer was only partially visible in the Ticker Edit Box.

		// Paint the last character:
		SelectObject(hdc, _hFont);
		SetBkColor(hdc, TickerEditBox::BKCOLOR);
		TextOut(hdc,
				_txtPix - _lastPartVisCharPix - xDelta,			// nXstart
				0, 												// nYstart
				_txtBuff + _lastPartVisChar,					// lpString
				1												// length of string
				);
		// Two subcases:
		// SUBCASE 2.1 - after scrolling the last character is still only partially visible.
		// SUBCASE 2.2 - after scrolling the last character is fully visible.
		if (lastCharWidth > xDelta + _lastPartVisCharPix)
		{
			// SUBCASE 2.1
			// No further drawing is necessary. Just update the state
			_txtPix = _wndWidth;
			_lastPartVisCharPix += xDelta;
		}
		else
		{
			// SUBCASE 2.2
			// update the state and then Fill Rect
			_txtPix = min(_wndWidth, _txtPix - xDelta - _lastPartVisCharPix + lastCharWidth);
			_lastPartVisCharPix = lastCharWidth;

			RECT r; r.left = _txtPix; r.top = 0; r.right = _wndWidth;r.bottom = _wndHeight;
			HBRUSH br = CreateSolidBrush(TickerEditBox::BKCOLOR);
			FillRect (hdc, &r, br);
			DeleteObject(br);
		}
	}
	else
	{
		// CASE 3 - the last character in the buffer was not visible at all in the Ticker Edit Box.
		// figure out how many char's are now visible behind the last (possibly only partially) visible char
		int paintOffPix = _wndWidth - _lastPartVisCharPix - xDelta;
		int paintBuffOff = _lastPartVisChar;

		// paint the characters that now are visible.
		SelectObject(hdc, _hFont);
		SetBkColor(hdc, TickerEditBox::BKCOLOR);
		TextOut(hdc,
				paintOffPix,									// nXstart
				0, 												// nYstart
				_txtBuff + paintBuffOff,						// lpString
				_txtBuffLen - paintBuffOff						// length of string
				);

		// SUBCASE 3.1 - now the entire characters are visible
		// SUBCASE 3.2 - there are still characters which are not visible to the right of the Ticker Edit Box

		int a = _lastPartVisCharPix + xDelta;
		int b = lastCharWidth;
		while (b < a && _lastPartVisChar < _txtBuffLen - 1)
		{
			_lastPartVisChar ++;
			b += CalcCharWidth(_txtBuff[_lastPartVisChar]);
		}

		if (b < a)
		{
			// SUBCASE 3.1 - the entire characters are visible.
			// update the state and then FillRect
			_txtPix = _wndWidth - a + b;
			_lastPartVisChar = _txtBuffLen - 1;
			_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);

			RECT r; r.left = _txtPix; r.top = 0; r.right = _wndWidth;r.bottom = _wndHeight;
			HBRUSH br = CreateSolidBrush(TickerEditBox::BKCOLOR);
			FillRect (hdc, &r, br);
			DeleteObject(br);
		}
		else
		{
			// SUBCASE 3.2 - there are still characters which are not visible to the right of the Ticker Edit Box.
			// in this case just update the state - no painting is necessary.
			_txtPix = _wndWidth;
			_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]) - b + a;
		}
	}

	// 4. Set Caret position and show the caret
	ShowCaret(_hWnd);
	SetCaretPos( _currXOffPix  /*X*/, TickerEditBox::caretYPos /*Y*/);
	// 5. Release the DC
	ReleaseDC(_hWnd, hdc);

	// 6. Done
}

//
// Process the delete action from the user.
//
void TickerEditBox::ProcessDelete()
{
	// there are no characters in the buffer or the caret is positioned at the beginning of the buffer.
	if (_txtBuffLen == 0 || _currBuffOff == 0)
	{
		return;
	}

	wchar_t delChar = _txtBuff[_currBuffOff-1];
	int delCharWidth = CalcCharWidth(delChar);

	// if we delete the char from the middle of the buffer, we have to shift the buffer to the left
	if (_currBuffOff < _txtBuffLen)
	{
		memmove(_txtBuff + _currBuffOff - 1 /*dest*/, _txtBuff + _currBuffOff /*src*/, sizeof(wchar_t) * (_txtBuffLen - _currBuffOff) /*num*/);
	}
	_txtBuffLen --; _txtBuff[_txtBuffLen] = L'\0';

	// There are a few cases:
	// CASE 1: user deletes the first (possibly only partially) visible character
	//		and there are no other characters in the buffer before the first (possibly only partially) visible character.
	//			In this case, shift the string to the left by the amount of the first character that is hidden.
	//			figure out if there are more characters to be seen past the right margin of the ticker edit box.
	//			Update the state.
	//			Set the caret at the beginning of the text box.
	//
	// CASE 2: user deletes the first (possibly only partially) visible character
	//		and there are other characters in the buffer before the first (possibly only partially visible character).
	//			in this case, bring into the ticker edit box navChars before the character the user wants to delete.
	//			Update the states.
	//			Set the caret to the right of the navCars.
	//
	// CASE 3: user deletes somewhere in the middle of the string
	//			in this case, shift the text to the left.
	//			figure out if we need to paint more (because now there is more room at the end of the string)
	//			Update the State.
	//			Set the caret to the left of the character the user just deleted

	if (_firstVisChar == _currBuffOff - 1 && _firstVisChar == 0)
	{
		// CASE 1. Delete the first character in the buffer

		int scrollLeftPix = delCharWidth - _firstVisCharHiddenPix;
		_currBuffOff --;
		_currXOffPix = 0;
		_firstVisChar = 0;
		_firstVisCharHiddenPix = 0;

		// User deleted a character before the last part vis char.
		_lastPartVisChar = max(0, _lastPartVisChar - 1);

		ScrollLeftWhenDelete(0 /*xDest*/, scrollLeftPix /*xSrc*/, scrollLeftPix /*xDelta*/);
	}
	else if (_firstVisChar == _currBuffOff - 1 && _firstVisChar > 0)
	{
		// CASE 2.
	}
	else
	{
		// CASE 3. user deletes a character somewhere in the middle of the string
		_currBuffOff --;
		_currXOffPix -= delCharWidth;

		// User deleted a character before the last part vis char.
		_lastPartVisChar = max(0, _lastPartVisChar - 1);

		ScrollLeftWhenDelete(_currXOffPix /*xDest*/, _currXOffPix + delCharWidth /*xSrc*/, delCharWidth /*xDelta*/);
	}
}

void TickerEditBox::ProcessKeyLeft()
{
	if (_currBuffOff == 0)
	{
		return;
	}

	if (_currBuffOff > _firstVisChar + 1 || (_currBuffOff == 1 && _firstVisChar == 0 && _firstVisCharHiddenPix == 0))
	{
		// The caret is somewhere in the middle of the string
		// OR
		// the first character in the string is fully visible and the caret is behind the first character in the string.
		// place the caret to the previous character and return.
		_currBuffOff --;
		_currXOffPix -= CalcCharWidth(_txtBuff[_currBuffOff]);
		SetCaretPos (_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	// This means that the caret is to the left of the first partially visible character
	// 		and that some portion of the first visible character is partially hidden.
	//
	// Shift the string to the right by the lesser amount of either navChar or _firstVisChar.
	// Re-position the caret to the left of what used to be the _firstVisCar.
	// _currBuffOff does not change, there will be the same number of characters in front of the caret.
	// We have to do some painting so we do our usual steps:
	//
	// 1. Hide the caret
	// 2. Get the DC from the hwnd
	// 3. Update State & Paint the character
	// 4. Set Caret position and show the caret
	// 5. Release the DC
	// 6. Done

	// 1. Hide the caret
	HideCaret(_hWnd);

	// 2. Get the DC from the hwnd
	HDC hdc = GetDC(_hWnd);
	// 3. Update State & Paint

	// how many characters are we going to shift?
	int shiftChar = min (_firstVisChar+1, TickerEditBox::navChar);
	int navCharsPix = 0;
	for (int i = 0; i < shiftChar; i++)
	{
		navCharsPix += CalcCharWidth(_txtBuff[_firstVisChar - i]);
	}

	// shift the string to the right by the amount that is partially hidden.
	int firstVisCharWidth = CalcCharWidth(_txtBuff[_firstVisChar]);
	BitBlt(
			hdc,											// destination HDC is the same as src HDC
			navCharsPix,									// xDest
			0,												// yDest
			max(0,_txtPix - _currXOffPix),					// nWidth
			_wndHeight,										// nHeight
			hdc,											// source HDC
			firstVisCharWidth - _firstVisCharHiddenPix,		// xSrc
			0,												// ySrc
			SRCCOPY											// Raster OP
			);
	// Paint the firstVisible char in its entirety
	SelectObject(hdc, _hFont);
	SetBkColor(hdc, TickerEditBox::BKCOLOR);
	TextOut(hdc,
			0,												// nXstart
			0, 												// nYstart
			_txtBuff + _firstVisChar - shiftChar + 1,		// lpString
			shiftChar										// length of string
			);

	// update our state
	_currXOffPix = navCharsPix; _firstVisChar -= shiftChar - 1;

	// update _lastPartVisChar.
	_lastPartVisChar = _firstVisChar;
	_txtPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
	while (_txtPix < _wndWidth && _lastPartVisChar < _txtBuffLen)
	{
		_lastPartVisChar ++;
		_txtPix += CalcCharWidth(_txtBuff[_lastPartVisChar]);
	}

	if (_txtPix > _wndWidth)
	{
		_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]) - _txtPix + _wndWidth;
		_txtPix = _wndWidth;
	}

	_firstVisCharHiddenPix = 0;

	// 4. Set Caret position and show the caret
	ShowCaret(_hWnd);
	SetCaretPos( _currXOffPix  /*X*/, TickerEditBox::caretYPos /*Y*/);
	// 5. Release the DC
	ReleaseDC(_hWnd, hdc);

	// 6. Done
}


//
// Right key navigation.
//
void TickerEditBox::ProcessKeyRight()
{
	// If we are already at the end of the string return
	if (_currBuffOff == _txtBuffLen)
	{
		return;
	}

	// when we move to the right the following cases are possible
	// CASE 1: the character to the right is entirely visible. Then just set the caret pos and return.
	// CASE 2: the character to the right is only partially visible (or totally invisible).
	// 			In this case, shift the string to the right by TickerEditBox::navChar.

	int charWidth = CalcCharWidth(_txtBuff[_currBuffOff]);

	if (_currXOffPix + charWidth <= _wndWidth)
	{
		// CASE 1: the character to the right is entirely visible. Then just set the caret pos and return.
		_currXOffPix += charWidth;
		_currBuffOff ++;
		SetCaretPos (_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	// CASE 2
	int shiftLeft;

	if (_lastPartVisCharPix < CalcCharWidth(_txtBuff[_lastPartVisChar]))
	{
		// the character to the right is only partially visible.
		shiftLeft = CalcCharWidth(_txtBuff[_lastPartVisChar]) - _lastPartVisCharPix;
	}
	else
	{
		// the character to the right is not visible at all (it is entirely invisible)
		_lastPartVisChar ++;
		shiftLeft = CalcCharWidth(_txtBuff[_lastPartVisChar]);
	}

	// We have to paint, so we follow the normal procedure:
	//
	// 1. Hide the caret
	// 2. Get the DC from the hwnd
	// 3. Update State & Paint
	// 4. Set Caret position and show the caret
	// 5. Release the DC
	// 6. Done

	// 1. Hide the caret
	HideCaret(_hWnd);

	// 2. Get the DC from the hwnd
	HDC hdc = GetDC(_hWnd);

	// 3. Update State & Paint
	BitBlt(
			hdc,								// destination HDC is the same as src HDC
			0,									// xDest
			0,									// yDest
			_wndWidth - shiftLeft,				// nWidth
			_wndHeight,							// nHeight
			hdc,								// source HDC
			shiftLeft,							// xSrc
			0,									// ySrc
			SRCCOPY								// Raster OP
			);
	// Paint the lastPartial Visible char in its entirety
	SelectObject(hdc, _hFont);
	SetBkColor(hdc, TickerEditBox::BKCOLOR);
	TextOut(hdc,
			_wndWidth - CalcCharWidth(_txtBuff[_lastPartVisChar]),		// nXstart
			0,															// nYstart
			_txtBuff + _lastPartVisChar,								// lpString
			1															// length of string
			);

	// update our state.
	// First the easy ones
	_currXOffPix = _wndWidth; _currBuffOff ++; _lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
	// the string now occupies the entire text box.
	_txtPix = _wndWidth;

	// we shifted to the left. So we have to update _firstVisChar and _firstVisCharHiddenPix
	int a = _firstVisCharHiddenPix + shiftLeft;
	int currWidth = CalcCharWidth(_txtBuff[_firstVisChar]);
	while (a > currWidth)
	{
		_firstVisChar ++;
		currWidth += CalcCharWidth(_txtBuff[_firstVisChar]);
	}

	_firstVisCharHiddenPix = CalcCharWidth(_txtBuff[_firstVisChar]) - currWidth + a;

	// 4. Set Caret position and show the caret
	ShowCaret(_hWnd);
	SetCaretPos( _currXOffPix  /*X*/, TickerEditBox::caretYPos /*Y*/);
	// 5. Release the DC
	ReleaseDC(_hWnd, hdc);

	// 6. Done
}

//
// navigation for when the user presses the Home Key
//
void TickerEditBox::ProcessKeyHome()
{
	// if the beginning of the string is visible then set the caret and return.
	// otherwise repaint everything, set the state and return.
	if (_firstVisChar == 0 && _firstVisCharHiddenPix == 0)
	{
		_currBuffOff = 0;
		_currXOffPix = 0;
		SetCaretPos( _currXOffPix  /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	// Paint the string, update the state and done.
	// The usual sequence:
	// 1. Hide the caret
	// 2. Get the DC from the hwnd
	// 3. Update State & Paint the character
	// 4. Set Caret position and show the caret
	// 5. Release the DC
	// 6. Done

	// 1. Hide the caret
	HideCaret(_hWnd);

	// 2. Get the DC from the hwnd
	HDC hdc = GetDC(_hWnd);
	// 3. Update State & Paint the character

	// Paint the ticker edit box.
	SelectObject(hdc, _hFont);
	SetBkColor(hdc, TickerEditBox::BKCOLOR);
	TextOut(hdc,
			0,								// nXstart
			0, 								// nYstart
			_txtBuff,						// lpString
			_txtBuffLen						// length of string
			);
	// set a whole bunch of stuff to 0
	_currBuffOff = 0; _currXOffPix = 0; _firstVisChar = 0; _firstVisCharHiddenPix = 0;

	// update _lastPartVisChar, _lastPartVisCharPix, _txtPix
	_lastPartVisChar = 0; _lastPartVisCharPix = 0; _txtPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
	while (_txtPix < _wndWidth && _lastPartVisChar < _txtBuffLen)
	{
		_lastPartVisChar ++;
		_txtPix += CalcCharWidth(_txtBuff[_lastPartVisChar]);
	}

	if (_txtPix > _wndWidth)
	{
		// we went over the right side of the text box.
		_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]) + _wndWidth - _txtPix;
		_txtPix = _wndWidth;
	}
	else
	{
		// we did not go over the right side of the text box.
		// fill the rest of the rectangle with the background color
		RECT r; r.left = _txtPix; r.top = 0; r.right = _wndWidth;r.bottom = _wndHeight;
		HBRUSH br = CreateSolidBrush(TickerEditBox::BKCOLOR);
		FillRect (hdc, &r, br);
		DeleteObject(br);

	}

	// 4. Set Caret position and show the caret
	ShowCaret(_hWnd);
	SetCaretPos( _currXOffPix  /*X*/, TickerEditBox::caretYPos /*Y*/);
	// 5. Release the DC
	ReleaseDC(_hWnd, hdc);

	// 6. Done
}

//
// navigation for when the user presses the End key
//
void TickerEditBox::ProcessKeyEnd()
{
	// If we are already at the end of the string return
	if (_currXOffPix == _txtPix)
	{
		return;
	}

	// if the end of the string is visible then set the caret and return
	if (_txtPix < _wndWidth)
	{
		_currBuffOff = _txtBuffLen;
		_currXOffPix = _txtPix;
		SetCaretPos(_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	// we have to shift the string to the left so that the end of the string is visible.
	// We have to paint, so we do the usual steps:
	//
	// 1. Hide the caret
	// 2. Get the DC from the hwnd
	// 3. Update State & Paint
	// 4. Set Caret position and show the caret
	// 5. Release the DC
	// 6. Done

	// 1. Hide the caret
	HideCaret(_hWnd);

	// 2. Get the DC from the hwnd
	HDC hdc = GetDC(_hWnd);

	// 3. Update State & Paint

	// first we have to figure out how much to paint.
	// we will have to measure how many characters the text box can fully paint. Starting from the end of the string;
	_txtPix = 0; _lastPartVisChar = _txtBuffLen - 1; _lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
	_firstVisChar = _lastPartVisChar; _firstVisCharHiddenPix = 0; _currBuffOff = _txtBuffLen;

	_txtPix = _lastPartVisCharPix;

	if (_firstVisChar > 0)
	{
		int nextCharWidth = CalcCharWidth(_txtBuff[_firstVisChar - 1]);
		while (_txtPix + nextCharWidth < _wndWidth && _firstVisChar > 0)
		{
			_txtPix += nextCharWidth;
			_firstVisChar --;
			nextCharWidth = CalcCharWidth(_txtBuff[_firstVisChar-1]);
		}
	}

	// Paint the string from the first visible char
	SelectObject(hdc, _hFont);
	SetBkColor(hdc, TickerEditBox::BKCOLOR);
	TextOut(hdc,
			0,												// nXstart
			0, 												// nYstart
			_txtBuff + _firstVisChar,						// lpString
			_lastPartVisChar - _firstVisChar + 1			// length of string
			);

	// Paint to the left of the last visible char
	RECT r; r.left = _txtPix; r.top = 0; r.right = _wndWidth;r.bottom = _wndHeight;
	HBRUSH br = CreateSolidBrush(TickerEditBox::BKCOLOR);
	FillRect (hdc, &r, br);
	DeleteObject(br);

	// update our internal state of the caret.
	_currXOffPix = _txtPix;


	// 4. Set Caret position and show the caret
	ShowCaret(_hWnd);
	SetCaretPos( _currXOffPix  /*X*/, TickerEditBox::caretYPos /*Y*/);
	// 5. Release the DC
	ReleaseDC(_hWnd, hdc);

	// 6. Done
}

//
// the user types a new character
//
void TickerEditBox::ProcessNewChar(WPARAM wParam)
{
	if (_txtBuffLen >= TickerEditBox::txtBuffSize - 1)
	{
		// The buffer is not enough for this user....
		return;
	}

	wchar_t newChar = (wchar_t) (wParam);

	// if we add the new char in the middle of the buffer, we have to shift the buffer to the right.
	if (_currBuffOff < _txtBuffLen)
	{
		memmove(_txtBuff + _currBuffOff + 1/*dest*/, _txtBuff + _currBuffOff /*src*/, sizeof(wchar_t) * (_txtBuffLen - _currBuffOff) /*num*/);
	}
	_txtBuff[_currBuffOff] = newChar; _txtBuffLen ++; _currBuffOff ++; _txtBuff[_txtBuffLen] = L'\0';

	int newCharWidth = CalcCharWidth(newChar);

	// Time to paint the character, update our state, position the caret
	// Here is the sequence of steps.
	//
	// 1. Hide the caret
	// 2. Get the DC from the hwnd
	// 3. Update State & Paint the character
	// 4. Set Caret position and show the caret
	// 5. Release the DC
	// 6. Done

	// 1. Hide the caret
	HideCaret(_hWnd);

	// 2. Get the DC from the hwnd
	HDC hdc = GetDC(_hWnd);

	// 3. Update State & Paint the character
	if (_currXOffPix + newCharWidth < _wndWidth)
	{
		// There is enough space between the caret and the right corner of the TickerEditBox window to paint the new character.
		// Shift the string painted to the right (if there is anything after the caret)
		if (_currBuffOff < _txtBuffLen)
		{
			BitBlt(
					hdc,								// destination HDC is the same as src HDC
					_currXOffPix + newCharWidth,		// xDest
					0,									// yDest
					_wndWidth - _currXOffPix - newCharWidth,			// nWidth
					_wndHeight,							// nHeight
					hdc,								// source HDC
					_currXOffPix,						// xSrc
					0,									// ySrc
					SRCCOPY								// Raster OP
					);
		}
		// Paint the new character.
		SelectObject(hdc, _hFont);
		SetBkColor(hdc, TickerEditBox::BKCOLOR);
		TextOut(hdc,
				_currXOffPix,									// nXstart
				0, 												// nYstart
				_txtBuff + _currBuffOff - 1,					// lpString
				1												// length of string
				);

		// update our state.
		_currXOffPix += newCharWidth;
		// The string shifted to the right. _firstVisChar and _firstVisCharHiddenPix do not change.
		// _lastPartVisChar and _lastPartVisCharPix have changed so they need to be updated.
		if (_currBuffOff < _txtBuffLen)
		{
			// the user typed in the middle of the string.

			if (_txtPix + newCharWidth <= _wndWidth)
			{
				// There was  enough space behind the last visible character to make room for
				// the new character.
				_lastPartVisChar = _txtBuffLen -1;
				_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
			}
			else
			{
				// There was not enough space behind the last visible character for the new character.
				// _lastPartVisChar increases by 1 (because we added a character in front of the last partially visible character)
				_lastPartVisChar ++;
				int a = _lastPartVisCharPix;
				while (a < newCharWidth)
				{
					_lastPartVisChar --;
					a += CalcCharWidth(_txtBuff[_lastPartVisChar]);
				}
				_lastPartVisCharPix = a - newCharWidth;
			}

		}
		else
		{
			// the user typed at the end of the string.
			_lastPartVisChar = _txtBuffLen - 1;
			_lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);
		}
		_txtPix = min(_wndWidth, _txtPix + newCharWidth);
	}
	else
	{
		// There is not enough space in the TickerEditBox to paint the new character.
		// Shift the string to the left.
		int shiftToLeftPix = newCharWidth - _wndWidth + _currXOffPix;

		BitBlt(
				hdc,								// destination HDC is the same as src HDC
				0,									// xDest
				0,									// yDest
				_wndWidth - shiftToLeftPix,			// nWidth
				_wndHeight,							// nHeight
				hdc,								// source HDC
				shiftToLeftPix,						// xSrc
				0,									// ySrc
				SRCCOPY								// Raster OP
				);
		// Paint the new character;
		SelectObject(hdc, _hFont);
		SetBkColor(hdc, TickerEditBox::BKCOLOR);
		TextOut(hdc,
				_wndWidth - newCharWidth,						// nXstart
				0, 												// nYstart
				_txtBuff + _currBuffOff - 1,					// lpString
				1												// length of string
				);

		// Update our state.
		_currXOffPix = _wndWidth;
		// The string shifted to the left.
		_lastPartVisChar ++; _lastPartVisCharPix = CalcCharWidth(_txtBuff[_lastPartVisChar]);

		// _firstVisChar and _firstVisCharHiddenPix changed so they need to be updated.
		int a = _firstVisCharHiddenPix + shiftToLeftPix;
		int currWidth = CalcCharWidth(_txtBuff[_firstVisChar]);
		while (a > currWidth)
		{
			_firstVisChar ++;
			currWidth += CalcCharWidth(_txtBuff[_firstVisChar]);
		}
		_firstVisCharHiddenPix = CalcCharWidth(_txtBuff[_firstVisChar]) - currWidth + a;

		// the string now occupies the entire text box.
		_txtPix = _wndWidth;
	}

	// 4. Show the caret and set position
	ShowCaret(_hWnd);
	SetCaretPos( _currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
	// 5. Release the DC
	ReleaseDC(_hWnd, hdc);

	// 6. Done
}


//
// Processes the WM_CHAR message
//
void TickerEditBox::ProcessWM_CHAR(WPARAM wParam)
{
	// we only process 0 ...9, a ... z, A ... Z, ' ', backspace (== L'\b') characters and Enter
	if (wParam == L'\r')
	{
		TickerEditBoxRefreshed(_txtBuff);
	}
	else if (wParam == L'\b')
	{
		// Delete operation.
		ProcessDelete();
	}
	else if ((wParam == L' ') ||
			 (wParam >= L'0' && wParam <= L'9') ||
			 (wParam >= L'a' && wParam <= L'z') ||
			 (wParam >= L'A' && wParam <= L'Z'))
	{
		ProcessNewChar(wParam);
	}
}


//
// Processes the WM_LBUTTONDOWN message
// Sets the caret where the user clicked in the text box.
//
void TickerEditBox::ProcessWM_LBUTTONDOWN(LPARAM lParam)
{
	if (_txtBuffLen == 0)
	{
		return;
	}

	int xPos = LOWORD(lParam);

	// a few special cases.

	if (xPos > _txtPix)
	{
		// user clicked to the right of the visible text.
		_currXOffPix = _txtPix;
		_currBuffOff = _txtBuffLen;
		SetCaretPos (_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	if (_txtBuffLen == _firstVisChar)
	{
		// the first (possibly only partially) visible character is the last character in the string.
		// Put the caret at the end of this character.
		_currXOffPix = CalcCharWidth(_txtBuff[_txtBuffLen]) - _firstVisCharHiddenPix;
		_currBuffOff = _txtBuffLen;
		SetCaretPos(_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	_currXOffPix = CalcCharWidth(_txtBuff[_firstVisChar]) - _firstVisCharHiddenPix;
	if (_currXOffPix > xPos)
	{
		// the user clicked on the visible portion of the first visible character.
		// put the caret at the end of this character.
		_currBuffOff = _firstVisChar;
		SetCaretPos (_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
		return;
	}

	// now the general case - the user clicked somewhere inside the text.
	_currBuffOff = _firstVisChar + 1;
	while (_currXOffPix < xPos)
	{
		_currBuffOff ++;
		_currXOffPix += CalcCharWidth(_txtBuff[_currBuffOff - 1]);
	}

	if ((_currXOffPix - xPos) * 2 > CalcCharWidth(_txtBuff[_currBuffOff - 1]))
	{
		_currXOffPix -= CalcCharWidth(_txtBuff[_currBuffOff - 1]);
		_currBuffOff --;

	}

	SetCaretPos (_currXOffPix /*X*/, TickerEditBox::caretYPos /*Y*/);
}


//
// Processes the WM_KEYDOWN message.
// Process VK_LEFT, VK_RIGHT, VK_HOME, VK_END.
//
void TickerEditBox::ProcessWM_KEYDOWN(WPARAM wParam)
{
	switch (wParam)
	{
	case VK_LEFT:
		{
			ProcessKeyLeft();
			return;
		}
	case VK_RIGHT:
		{
			ProcessKeyRight();
			return;
		}
	case VK_HOME:
		{
			ProcessKeyHome();
			return;
		}

	case VK_END:
		{
			ProcessKeyEnd();
			return;
		}
	}
}


TickerEditBox::~TickerEditBox()
{
	// Nothing to do here. We cleanup our resources in the Cleanup() function.
}

//
// The Window Proc for the Ticker Edit Box
//
LRESULT CALLBACK TickerEditBox::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
			tickerEB->Cleanup();
			PostQuitMessage(0);
			return 0;
		}
    case WM_CREATE:
		{
			CREATESTRUCT * cs = reinterpret_cast <CREATESTRUCT *> (lParam);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) cs->lpCreateParams);
			return 0;
		}
    case WM_LBUTTONDOWN:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
			tickerEB->ProcessWM_LBUTTONDOWN(lParam);
			return 0;
		}
    case WM_KEYDOWN:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
			tickerEB->ProcessWM_KEYDOWN(wParam);
			return 0;
		}
    case WM_CHAR:
    	{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
			for (int i = 0; i < LOWORD(lParam); i++)
			{
				tickerEB->ProcessWM_CHAR(wParam);
			}

			return 0;
    	}
    case WM_PAINT:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));

			PAINTSTRUCT ps;
            HDC hdc = BeginPaint (hwnd, &ps);

			// paint our string
			SelectObject(hdc, tickerEB->_hFont);
			SetBkColor(hdc, TickerEditBox::BKCOLOR);
			TextOut(hdc,
					0,												// nXstart
					0, 												// nYstart
					tickerEB->_txtBuff, 							// lpString
					tickerEB->_txtBuffLen							// length of string
					);

			EndPaint(hwnd, &ps);
			return 0;
		}
    case WM_SETFOCUS:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
    		CreateCaret(tickerEB->_hWnd, NULL, 1, tickerEB->_wndHeight - TickerEditBox::caretYPos * 1 - 1);
    		SetCaretPos (tickerEB->_currXOffPix, TickerEditBox::caretYPos);
    		ShowCaret (tickerEB->_hWnd);

    		return 0;
		}
    case WM_KILLFOCUS:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
			HideCaret (tickerEB->_hWnd);
			DestroyCaret();

			return 0;
		}
    case WM_SIZE:
		{
			TickerEditBox * tickerEB = reinterpret_cast <TickerEditBox *> (GetWindowLongPtr(hwnd, GWLP_USERDATA));
            tickerEB->_wndWidth = LOWORD (lParam);
            tickerEB->_wndHeight = HIWORD (lParam);
            tickerEB->ResetTextBoxState();

            return 0;
		}
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
