COMPILER = g++
LINKER = g++
C_FLAGS = -c -O0 -g3 -Wall -fmessage-length=0
LIBS = -lgdi32 -lole32 -lWs2_32 -lMswsock
LIB_PATH = -L"C:\MinGW\lib"

#LINK_FLAGS = -static -mconsole
LINK_FLAGS = -static -mwindows


all : Debug-make Ticker\StockTicker.exe

clean :
	rm -rf Ticker
	echo Clean done

Debug-make : 
	mkdir -p Ticker

Ticker\WinMain.o: WinMain.cpp WinFrame.h Ticker.h TickerEditBox.h WorkerThread.h
	$(COMPILER) $(C_FLAGS) -o Ticker\WinMain.o -c WinMain.cpp

Ticker\WinFrame.o: WinFrame.cpp WinFrame.h Ticker.h TickerEditBox.h WorkerThread.h
	$(COMPILER) $(C_FLAGS) -o Ticker\WinFrame.o -c WinFrame.cpp

Ticker\TickerTape.o: TickerTape.cpp TickerTape.h Ticker.h
	$(COMPILER) $(C_FLAGS) -o Ticker\TickerTape.o -c TickerTape.cpp

Ticker\TickerEditBox.o: TickerEditBox.cpp WinFrame.h Ticker.h TickerEditBox.h WorkerThread.h
	$(COMPILER) $(C_FLAGS) -o Ticker\TickerEditBox.o -c TickerEditBox.cpp

Ticker\WorkerThread.o: WorkerThread.cpp WorkerThread.h WinFrame.h Ticker.h TickerEditBox.h WorkerThread.h
	$(COMPILER) $(C_FLAGS) -o Ticker\WorkerThread.o -c WorkerThread.cpp

Ticker\StockTicker.exe : Ticker\WinMain.o Ticker\WinFrame.o Ticker\TickerTape.o Ticker\TickerEditBox.o Ticker\WorkerThread.o
	$(LINKER) -o Ticker/StockTicker.exe \
		Ticker/WinMain.o \
		Ticker/WinFrame.o \
		Ticker/TickerTape.o \
		Ticker/TickerEditBox.o \
		Ticker/WorkerThread.o \
		$(LIBS) $(LIB_PATH) $(LINK_FLAGS) 
	echo Build done

	
