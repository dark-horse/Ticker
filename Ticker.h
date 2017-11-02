/*
 * Ticker.h
 *
 * Main header file for the program. Contains the linkages to Win32
 * and a few definitions used throughout the program.
 *
 */

#ifndef TICKER_H_
#define TICKER_H_

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT            // Minimum platform is Windows XP
#define _WIN32_WINNT 0x0501
#endif

#define WIN32_LEAN_AND_MEAN     // For WinSock. Needs to be defined before including <windows.h>

#include <windows.h>

// SUCCESS / ERROR CODES

#define FUNC_RESULT int

#define SUCCESS 								0x00000000

#define ERR_OUT_OF_MEMORY						0x00000001

#define ERR_WINDOW_NOT_REGISTERED				0x00000002
#define ERR_WINDOW_NOT_CREATED  				0x00000004
#define ERR_TICKERBAND_FONT_NOT_CREATED			0x00000008
#define ERR_TICKER_HDC_NOT_CREATED				0x00000010
#define ERR_GET_TEXT_METRICS_FAIL				0x00000020
#define ERR_TICKER_TAPE_PAINT_FAIL				0x00000040
#define ERR_GET_CHAR_WIDTHS_FAIL				0x00000080
#define ERR_BK_BRUSH_NOT_CREATED				0x00000100
#define ERR_TICKER_EB_FONT_NOT_CREATED			0x00000200

// Constants
#define StockTickerBackgroundColor RGB(255,255,153)

#endif /* TICKER_H_ */
