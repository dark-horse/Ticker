Ticker
======

Disclaimer 

My friend said I should put this disclaimer:

The program and source code contained in this repository are for general usage purposes only. We make no representations or warranties of any kind, express or implied, about the completeness, accuracy, reliability, suitability or availability with respect to program and the source code contained in this repository. Any reliance you place on such information is therefore strictly at your own risk.
In no event will we be liable for any loss or damage including without limitation, indirect or consequential loss or damage, or any loss or damage whatsoever arising from loss of data or profits arising out of, or in connection with, the use / misuse / abuse of this program and source code.

[In other words, this is the Internet and you are using something that someone called The Dark Horse uploaded on a free site. Try explaining that to your boss / wife / etc...]

Now for the fun part.

1. This program is written by a hobbyist to learn / play with C++ and Win32.
2. The program is clearly an exercise in C++ / Win32 (For a program with such a simple functionality
      there was no need to re-invent the Edit control. Or encapsulate the two Windows in the program
      in their own class).
3. You can use parts of this program and / or source code however you want.
4. The program displays stock data retrieved from Yahoo! Finance. Right now Yahoo! Finance provides this
      free of charge. If Yahoo! Finance changes its mind about this, then this program will not work.
5. There is no help file for this program. If you want to use the program, start typing stock tickers in the
      text box at the bottom of the program (up to 1024 characters). When you entered all the 
      stock tickers you want, hit Enter. The program will then call into Yahoo! Finance and, when the data
      is available, will show the stock prices for the tickers.
6. There is no input validation. If the user types the wrong stock ticker symbol the
       program will display the ticker symbol with no stock price. The user will learn a lesson!!!