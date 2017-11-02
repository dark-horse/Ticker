/*
 * TickerEditBox.h
 *
 *  This is the "custom made" edit control for the Stock Ticker program.
 *  Will allow the user to enter the ticker symbol for the quotes s/he is
 *  	interested in.
 *
 */

#ifndef TICKEREDITBOX_H_
#define TICKEREDITBOX_H_

#include "Ticker.h"

class WinFrame;					// forward declaration of WinFrame class.

class TickerEditBox
{
public:
	TickerEditBox();
	FUNC_RESULT CreateWindowClass(HINSTANCE hInstance);
	FUNC_RESULT InitTickerEditBox(WinFrame *parentFrame, HWND winFrameHWnd, HFONT winFrameTickerEBFont, int x, int y, int nWidth, int nHeight);
	void SetPosition(int x, int y, int nWidth, int nHeight);
	void ShowTickerEditBox();
	FUNC_RESULT HideTickerEditBox();
	void TickerEditBoxRefreshed(LPCWSTR tickedEditBoxText);
	virtual ~TickerEditBox();


private:
	// Consts
	//
	static const wchar_t CLASS_NAME[];
	static const COLORREF BKCOLOR= RGB(250,250,250);

	// Constants for text stuff
	static const int txtBuffSize = 1024;
	static const wchar_t firstChar = L' ';
	static const wchar_t lastChar = L'~';

	static const int caretYPos = 2;

	static const int navChar = 5;		// When the caret is to the left of the first (possibly only partially) visible character
										// and the user presses the left key, attempt to navigate to the left by navChar.
										// and the user presses the delete key, then delete the character and shift navChar into view.


	// Member variables
	//
	HINSTANCE _hInstance;
	WNDCLASSEX _wc;
	HWND _winFrameHWnd;
	HWND _hWnd;
	HFONT _hFont;						// WinFrame will give us this font. Do not destroy it, WinFrame controls the lifetime of this font.
	WinFrame *_parentFrame;				// The program win frame. The TickerEditBox has to let win frame know that the user finished typing.
	int _wndWidth;
	int _wndHeight;

	// Member variables for Text manipulation
	ABC _ABCWidths [(UINT)lastChar - (UINT)firstChar + 1];		// cache the widths of the characters the Ticker Box will accept
	wchar_t _txtBuff[txtBuffSize];								// our text buffer.
	int _txtBuffLen;											// length of the text buffer (so we don't have to call wcslen all over the place)
	int _currBuffOff;											// number of characters to the left of the caret.
	int _currXOffPix;											// number of pixels to the left of the caret. Measured from the left corner of the edit box
	int _firstVisChar;											// index of the first character that is partially visible in the text box
	int _firstVisCharHiddenPix;									// the numbers of pixels of the _firstVisChar which are not visible (are hidden).
	int _lastPartVisChar;										// index of the last char that is partially visible in the text box
	int _lastPartVisCharPix;									// the number of visible pixels of the last partially visible char
	int _txtPix;												// the number of pixels the text takes in the edit box;



	// The WindowProc for the Ticker Edit Box. Needs to be static
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


	// Member functions
	//
	void Cleanup();
	void InitCharacterWidthsCache();
	int CalcCharWidth(wchar_t inputChar);
	void ResetTextBoxState();
	void PrintState();
	void ScrollLeftWhenDelete(int xDest, int xSrc, int xDelta);
	void ProcessDelete();
	void ProcessKeyLeft();
	void ProcessKeyRight();
	void ProcessKeyHome();
	void ProcessKeyEnd();
	void ProcessNewChar(WPARAM wParam);
	void ProcessWM_CHAR(WPARAM wParam);
	void ProcessWM_LBUTTONDOWN(LPARAM lParam);
	void ProcessWM_KEYDOWN(WPARAM wParam);
};

#endif /* TICKEREDITBOX_H_ */
