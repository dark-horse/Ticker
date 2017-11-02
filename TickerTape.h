/*
 * TickerTape.h
 *
 * This class will paint the stock prices smoothly scrolling the Stock Quotes
 * 		across the main window.
 *
 * 	Similar to the paper strip (tape) which came out of the telegraphic printer.
 *
 */

#ifndef TICKERTAPE_H_
#define TICKERTAPE_H_

#include "Ticker.h"

class TickerTape
{
public:

	// Functions
	TickerTape();
	virtual ~TickerTape();
	FUNC_RESULT Init();
	FUNC_RESULT PaintTickerTape(int offset, HDC destHdc, int clientWidth, int clientHeight);
	void UpdateText(LPCWSTR newText);
	int GetTotalWidth();
	void Cleanup();

private:
	// Constants
	static const int TRAILING_SPACE = 20;
	static const int txtBuffSize = 1024;

	// Member Variables
	HDC _tickerHdc;
	HBITMAP _tickerHBitmap;
	HFONT _hFont;
	HBRUSH _hBrush;
	int _tapeHeight;
	int _tapeWidth;
	wchar_t _stringToPaint [txtBuffSize];

	// functions
	FUNC_RESULT CreateMemoryDC();
	FUNC_RESULT GetTickerBandSize (HDC hdc, LPCWSTR paintString, int *width, int *height);
};

#endif /* TICKERTAPE_H_ */
