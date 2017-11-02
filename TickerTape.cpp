/*
 * TickerTape.cpp
 *
 */

#include "TickerTape.h"

TickerTape::TickerTape()
{
	_tickerHdc = NULL;
	_hFont = NULL;
	_tickerHBitmap = NULL;
	_hBrush = NULL;
}


//
// The function creates the Font to draw with, the memory DC and paints the text we want to paint into the memory DC
//
FUNC_RESULT TickerTape::Init()
{
	 // Create the font
	 _hFont = CreateFont
			 	 (
	             		60,						// Height
	             		0,						// Width - go with Windows default
	             		0,						// Escapement
	             		0,						// Orientation
	             		FW_BOLD,				// Weight
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

	 if (!_hFont) return ERR_TICKERBAND_FONT_NOT_CREATED;
	 _hBrush = CreateSolidBrush (StockTickerBackgroundColor);
	 if (!_hBrush) return ERR_BK_BRUSH_NOT_CREATED;

	 int strLen = wcslen (L"Updated the string!");
	 wcsncpy(_stringToPaint/*dest*/, L"Updated the string!"/*src*/, min (strLen, TickerTape::txtBuffSize-1)/*num*/);
	 _stringToPaint[min(strLen, TickerTape::txtBuffSize-1)] = L'\0';

	 if (this->CreateMemoryDC() != SUCCESS) return ERR_TICKER_HDC_NOT_CREATED;

	 return SUCCESS;
}

//
// Function that paints into the destHDC
//
FUNC_RESULT TickerTape::PaintTickerTape(int offset, HDC destHdc, int clientWidth, int clientHeight)
{
	FUNC_RESULT result = SUCCESS;

	if (_tickerHdc == NULL || _hFont == NULL || _tickerHBitmap == NULL)
		result = ERR_TICKER_TAPE_PAINT_FAIL;

	// There are two modes to paint the ticker.
	// 1. 	When the width of the ticker is larger than the client Width.
	//		In this case, we start painting the beginning of the ticker string as soon
	//		as we finished painting the trailing spaces.
	//
	// 2.	When the width of the ticker is smaller than the client width.
	//		In this case we start painting the beginning of the wrap-around ticker string
	//		right away.
	//

	if (GetTotalWidth() > clientWidth)
	{
		// Mode 1.
		// Start painting what is left of the ticker string
		int pixelsPainted = min(_tapeWidth - offset, clientWidth);
		BitBlt(
				destHdc,							// destination HDC
				0,									// xDest
				(clientHeight - _tapeHeight)/2,		// yDest
				pixelsPainted,						// nWidth
				_tapeHeight,						// nHeight
				_tickerHdc,							// source HDC
				offset,								// xSrc
				0,									// ySrc
				SRCCOPY								// Raster OP
				);

		// If there is space left, paint the trailing spaces
		if (pixelsPainted < clientWidth)
		{
			RECT r; r.top = (clientHeight - _tapeHeight)/2; r.left = pixelsPainted; r.bottom = (clientHeight + _tapeHeight)/2; r.right = r.left + TickerTape::TRAILING_SPACE;
			FillRect(destHdc, &r, _hBrush);
			pixelsPainted += TickerTape::TRAILING_SPACE;
		}

		// If there is space left after painting the trailing spaces, BitBlt the beginning of the ticker string
		if (pixelsPainted < clientWidth)
		{
			BitBlt(
					destHdc,							// destination HDC
					pixelsPainted,						// xDest
					(clientHeight - _tapeHeight)/2,		// yDest
					clientWidth - pixelsPainted,		// nWidth
					_tapeHeight,						// nHeight
					_tickerHdc,							// source HDC
					0,									// xSrc
					0,									// ySrc
					SRCCOPY								// Raster OP
					);
		}
	}
	else
	{
		// Mode 2.
		int pixelsPainted = 0;
		// If there is still a portion of the initial ticker string on the screen, paint it.
		if (offset < _tapeWidth)
		{
			pixelsPainted += _tapeWidth - offset;
			BitBlt(
					destHdc,							// destination HDC
					0,									// xDest
					(clientHeight - _tapeHeight)/2,		// yDest
					pixelsPainted,						// nWidth
					_tapeHeight,						// nHeight
					_tickerHdc,							// source HDC
					offset,								// xSrc
					0,									// ySrc
					SRCCOPY								// Raster OP
					);
		}
		// paint the background all the way to the beginning of the wrap-around portion of the ticker tape
		RECT r; r.top = (clientHeight - _tapeHeight)/2; r.left = pixelsPainted; r.bottom = (clientHeight + _tapeHeight)/2; r.right = clientWidth - offset;
		FillRect (destHdc, &r, _hBrush);
		pixelsPainted = clientWidth - offset;

		// paint the wrap-around portion of the ticker tape
		BitBlt(
				destHdc,							// destination HDC
				clientWidth - offset,				// xDest
				(clientHeight - _tapeHeight)/2,		// yDest
				_tapeWidth,							// nWidth
				_tapeHeight,						// nHeight
				_tickerHdc,							// source HDC
				0,									// xSrc
				0,									// ySrc
				SRCCOPY								// Raster OP
				);
		pixelsPainted += _tapeWidth;
		// if there is space behind the wrap around fill with the Brush
		if (pixelsPainted < clientWidth)
		{
			RECT r; r.top = (clientHeight - _tapeHeight)/2; r.left = pixelsPainted; r.bottom = (clientHeight + _tapeHeight)/2; r.right = clientWidth;
			FillRect (destHdc, &r, _hBrush);
		}
	}

	return result;
}

//
// We have to update the ticker text.
//
void TickerTape::UpdateText(LPCWSTR newText)
{
	int strLen = wcslen(newText);
	strLen = min (strLen, TickerTape::txtBuffSize-1);
	wcsncpy(_stringToPaint/*dest*/, newText/*src*/, strLen/*num*/);
	_stringToPaint[strLen] = L'\0';

	// destroy the current HDC and bitmap and recreate new ones
	if (_tickerHdc) DeleteDC (_tickerHdc);
	_tickerHdc = NULL;

	if (_tickerHBitmap) DeleteObject (_tickerHBitmap);
	_tickerHBitmap = NULL;

	CreateMemoryDC();
}

//
// Returns the total width of the ticker band = text + padding.
//
int TickerTape::GetTotalWidth()
{
	return _tapeWidth + TickerTape::TRAILING_SPACE;
}

//
// Clean up after yourself.
// NOTE - Petzold and MSDN seem to imply that the hBitmp should be deleted after the Hdc
//
void TickerTape::Cleanup()
{
	if (_tickerHdc) DeleteDC (_tickerHdc);
	_tickerHdc = NULL;

	if (_hFont) DeleteObject (_hFont);
	_hFont = NULL;

	if (_tickerHBitmap) DeleteObject (_tickerHBitmap);
	_tickerHBitmap = NULL;

	if (_hBrush) DeleteObject (_hBrush);
	_hBrush = NULL;
}


//
// This functions does a few things:
// 		It creates a Memory DC which has the same color organization as the DISPLAY DC
// 		It calculates the width / height of the ticker band.
//		It paints the text into the Memory DC.
//
// When the function returns, the memory DC is ready to be bit blt'ed into the win frame device context
//
FUNC_RESULT TickerTape::CreateMemoryDC()
{
	FUNC_RESULT result = SUCCESS;

	// Steps to create the memory DC
	// 1. Figure out what hdc should be the source for the internal bitmap and the internal dc.
	// 2. CreateCompatibleDC. Question - what hdc should be the source for this???
	// 3. Figure out the size of the bitmap. Save it for later.
	// 4. Create a bitmap with the size we want and the color organization as the source HDC.
	// 5. Set our internal bitmap into our internal DC. Use the SelectObject function for this.
	// 6. Paint our text into this DC.
	// 7. Cleanup.
	// 8. DONE!

	// 1. Figure out the hdc to use as the source. From Petzold - use the DISPLAY DC.
	HDC displayHDC = CreateDC (L"DISPLAY", NULL, NULL, NULL);
	if (!displayHDC) result = ERR_TICKER_HDC_NOT_CREATED;

	// 2. CreateCompatible DC
	if (result == SUCCESS)
	{
		_tickerHdc = CreateCompatibleDC(displayHDC);
		if (!_tickerHdc) result = ERR_TICKER_HDC_NOT_CREATED;
	}

	// 3. Figure out the size of the bitmap. Save it for later.
	int height, width;
	if (result == SUCCESS)
	{
		// We have to select our font into the displayHDC. Seems kind of hacky...
		SelectObject(displayHDC, _hFont);
		result = GetTickerBandSize (displayHDC, _stringToPaint, &width, &height);
	}
	_tapeHeight = height; _tapeWidth = width;


	// 4. Create a bitmap with the size we want and the color organization as the source HDC.
	if (result == SUCCESS)
	{
		_tickerHBitmap = CreateCompatibleBitmap(displayHDC, width, height);
		if (!_tickerHBitmap) result = ERR_TICKER_HDC_NOT_CREATED;
	}

	// 5. Set the internal bitmap into the memory DC
	if (result == SUCCESS)
	{
		SelectObject(_tickerHdc, _tickerHBitmap);
	}

	// 6. Paint our text into the memory DC

	// Sometimes TextOut does fill in the entire rectangle that is calculated with GetTickerBandSize
	// One solution would be to not use TextOut - and paint each character at a time in its own rectangle.
	// Another solution would be to use TextOut - and FillRect the last few pixels. We do this here.
	RECT r; r.left = _tapeWidth - 20; r.top = 0; r.right = _tapeWidth; r.bottom = _tapeHeight;
	FillRect (_tickerHdc, &r, _hBrush);

	// we select the _hFont into the memory DC
	//
	if (result == SUCCESS)
	{
		if (!SelectObject (_tickerHdc, _hFont)) result = ERR_TICKER_HDC_NOT_CREATED;
	}
	// then we set the background color for the text
	//
	if (result == SUCCESS)
	{
		if (SetBkColor(_tickerHdc, StockTickerBackgroundColor) == CLR_INVALID) result = ERR_TICKER_HDC_NOT_CREATED;
	}

	// now we paint the text
	//
	if (result == SUCCESS)
	{
		int strLen = wcslen (_stringToPaint);
		if (!TextOut(_tickerHdc,
				  0,												// nXstart
				  0, 												// nYstart
				  _stringToPaint, 									// lpString
				  strLen											// length of string
				  )
		) result = ERR_TICKER_HDC_NOT_CREATED;
	}

	// 7. Cleanup. We only need to cleanup the display HDC
	if (displayHDC) DeleteDC (displayHDC);

	// 8. DONE!
	return result;
}

//
// INPUTS: a device context and the string we want to display.
// OUTPUTS: the height and the width of the smallest rectangle that will fit the string on the screen.
//
FUNC_RESULT TickerTape::GetTickerBandSize(HDC hdc, LPCWSTR paintString, int *width, int *height)
{
	// Height is relatively easy: use GetTextMetrics
	//
	TEXTMETRIC tm;
	if (!GetTextMetrics(hdc, &tm)) return ERR_GET_TEXT_METRICS_FAIL;
	*height = tm.tmHeight;

	// width is more work. We have to get the widths for each character.
	// we will use the GetCharABCWidths function for the following ranges of characters:
	//
	//		' ' ... '~'
	//
	// This range of characters includes the numerals (0...9) lower case and upper case letter (a...z, A...Z)
	// and most of the special characters (including all characters we will use in our display)

	ABC abc_ABCWidths [(UINT)L'~' - (UINT)L' '+1];
	if (!GetCharABCWidths(hdc, (UINT) L' ', (UINT) L'~', abc_ABCWidths)) return ERR_GET_CHAR_WIDTHS_FAIL;

	// Now loop through the characters in the string and calculate the width
	int width_calc = 0;

	int strLen = wcslen(paintString);
	for (int i = 0; i < strLen; i++)
	{
		wchar_t curr = paintString[i];
		if (curr >= L' ' && curr <= L'~')
		{
			ABC abc = abc_ABCWidths[ (UINT) curr - (UINT) ' '];
			width_calc += abc.abcA + abc.abcB + abc.abcC;
		}
		else
		{
			return ERR_GET_CHAR_WIDTHS_FAIL;
		}
	}

	*width = width_calc;
	return SUCCESS;
}

TickerTape::~TickerTape()
{
	// TODO Auto-generated destructor stub
}

