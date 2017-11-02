/*
 * WorkerThread.h
 *
 * RULE OF THUMB: UI Thread sends / gets data to / from Worker Thread when WorkerThread state is blocked on the waitingOnWork event.
 *
 */

#ifndef WORKERTHREAD_H_
#define WORKERTHREAD_H_

#include "Ticker.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>

class WorkerThread
{
public:
	WorkerThread();

	FUNC_RESULT Init();														// Initialization sequence for the Worker Thread.
																			// Worker Thread is not started yet.
																			// Method will execute on the UI Thread.

	void UIThreadRequestsStockQuotes(const wchar_t *pStockSymbols);			// UI thread is looking to get stock quotes from the Internet.
																			// Will copy the stock symbols into the WorkerThread buffer.
																			// Will execute on the UI thread and the Worker Thread MUST be blocked waiting for work.

	void UIThreadUpdateWithStockQuotes(wchar_t *pStockQuotes, int len);		// UI thread is updated with the stock quotes we pulled from the Internet.
																			// Will copy the stock quotes into the UI thread buffer.
																			// Will execute on the UI thread and the Worker Thread MUST be blocked waiting for work.

	void UIThreadRequestsEndThread();										// UI thread is winding down. Time to end the Worker Thread
																			// Returns the end thread requested event and UI thread will wait on it.

	BOOL UIThreadWorkerThreadBlocked();										// UI Thread wants to check if the WaitingForWork event is signaled or not.

	virtual ~WorkerThread();

private:

	// Private Consts
	//

	// Constants for connecting to Yahoo Finance
	//
	static const char HOST[];						// something along the lines "download.finance.yahoo.com"
	static const char DEFAULT_PORT[];				// The HTTP port on finance.yahoo.com port 80;
	static const char requestBegin[];				// something along the lines "GET /d/quotes.cvs?s="
	static const char requestEnd[];					// something along the lines "&f=sl1&e=.csv\r\n"

	// Some Error messages (it would be cleaner to use error codes for communication with
	//						the UI, but I am feeling lazy right now. Maybe a later version.)
	//
	static const char CannotConnectToInternet[];	// "Cannot Connect to Internet."
	static const char ConnectionDropped[];			// "Connection Dropped. Will try again later."
	static const char InternetRequestDidNotWork[];	// "Connection Unsuccessful. Will try again later."

	// Text Buffer Constants.
	//
	static const int sendBuffLen = 1024;			// the size of the request buffer we sent to the Internet.
	static const int recvBuffLen = 1024;			// the size of the buffer we got back from the Internet.
	static const int stockSymBuffLen = 1024;		// the size of the buffer that has the stock ticker symbols
	static const int stockQuotesBuffLen = 1024;		// the size of the wide char buffer with the stock quotes information ready to be displayed.

	static const long endThreadNotRequested = 0;	// 0 means that the UI did not request to end the WorkerThread.
	static const long endThreadRequested = 1;		// 1 means the opposite.


	// Private Member Variables
	//

	// Text buffers to process requests / results to / from Internet
	//
	char 		_sendBuff[sendBuffLen];					// the request text we send out. Internet works with chars, not wide chars.
	char 		_recvBuff[recvBuffLen];					// the result we get from the Internet. Winsock gives us back chars, not wide chars.
	wchar_t 	_stockSymBuff[stockSymBuffLen];			// the stock ticker symbols we want to get the prices for. We get this from the UI thread.
	wchar_t 	_stockQuotesBuff[stockQuotesBuffLen];	// the result we got from the Internet in a form ready to display to the user.

	// WinSock variables
	//
	struct addrinfo *_addrInfo;						// convert a string to a xxx.xxx.xxx.xxx id
	SOCKET			 _connectSocket;				// our connection to the Internet.
	BOOL 			 _wsaInitializedSucc;			// so we know to call WSACleanup.
	BOOL 			 _addrInitializedSucc;			// getaddrinfo succeeded.
	BOOL 			 _socketConnected;				// TRUE when the socket is successfully connected to the Internet.

	// Synchronization Variables
	//
	long _UIRequestedEndThread;						// use a long variable because we will have to use InterlockedCompareExchange
													// 0 means that the UI did not request to end the WorkerThread.
													// 1 means the opposite.

	HANDLE _waitForWorkEvent;						// Worker thread waits for work on this signal.
	HANDLE _endThreadRequestedEvent;				// The UI Thread waits on this signal for the Worker Thread to finish.


	// Private Functions
	//
	void CreateInternetRequest();					// creates the request buffer to send to the Internet.
	void CreateInternetResult();					// creates the stock quotes buffer from the result we got from the internet.
	BOOL InitializeInternetConnection();			// Initializes the Internet connection (WinSock WSA, addrInfo);
	void InitializeSocket();						// We may need to initialize the socket multiple times during the process (say, because of dropped connections).
													// So have a function for doing just this.
	void GetData();									// The function that gets the data from the Internet.
	void Cleanup();									// cleans up our sockets, connections, WSA, etc..
	void WorkLoop();								// the work loop.
	static void Thread(PVOID pvoid);				// Thread entry point. Needs to be static.
};

#endif /* WORKERTHREAD_H_ */
