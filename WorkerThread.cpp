/*
 * WorkerThread.cpp
 *
 */

#include "WorkerThread.h"

const char WorkerThread::DEFAULT_PORT[] = "80";
const char WorkerThread::HOST[] = "download.finance.yahoo.com";
const char WorkerThread::requestBegin[] = "GET /d/quotes.cvs?s=";
const char WorkerThread::requestEnd[] = "&f=sl1&e=.csv\r\n";
const char WorkerThread::CannotConnectToInternet[] = "Cannot Connect to Internet.";
const char WorkerThread::ConnectionDropped[] = "Connection Dropped. Will try again later.";
const char WorkerThread::InternetRequestDidNotWork[] = "Connection Unsuccessful. Will try again later.";

WorkerThread::WorkerThread()
{
}

//
// The UI is sending us a buffer of stock symbols.
// Called from the UI Thread.
//
void WorkerThread::UIThreadRequestsStockQuotes(const wchar_t *pStockSymbols)
{
	int a = min (wcslen(pStockSymbols), WorkerThread::stockSymBuffLen-1);
	if (a == 0)
	{
		return;
	}

	wcsncpy(_stockSymBuff/*dest*/, pStockSymbols/*src*/, a /*num*/);
	_stockSymBuff[a] = L'\0';

	SetEvent(_waitForWorkEvent);
}

//
// The UI thread is updated with the Stock Quotes we pulled from the Internet.
// Called from the UI thread.
//
void WorkerThread::UIThreadUpdateWithStockQuotes(wchar_t *pStockQuotes, int len)
{
	int a = min (wcslen(_stockQuotesBuff), len-1);
	wcsncpy(pStockQuotes /*dest*/, _stockQuotesBuff /*src*/, a);
	*(pStockQuotes + a) = L'\0';
}

//
// UI Thread tells the Worker Thread that Worker Thread should be terminated.
// Called from the UI Thread.
//
void WorkerThread::UIThreadRequestsEndThread()
{
	InterlockedExchange(&_UIRequestedEndThread, 1);

	// unblock the worker thread
	SetEvent(_waitForWorkEvent);

	// Wait on the event.
	WaitForSingleObject(_endThreadRequestedEvent, INFINITE);
}

//
// UI Thread wants to know if the WorkerThread is blocked.
// Called from the UI Thread.
// Non Blocking call to WaitForSingleObject
//
BOOL WorkerThread::UIThreadWorkerThreadBlocked()
{
	return (WaitForSingleObject(_waitForWorkEvent, 0) == WAIT_TIMEOUT);
}

//
// takes the stock symbols we got from the UI and
// creates the request buffer to send to Internet.
//
void WorkerThread::CreateInternetRequest()
{
	memset (_sendBuff, 0, sizeof(_sendBuff));
	char *t = _sendBuff;

	int a = strlen(WorkerThread::requestBegin);
	int b = strlen(WorkerThread::requestEnd);

	memcpy(_sendBuff/*dest*/, WorkerThread::requestBegin/*src*/, a /*num*/);
	t += a;

	int stockSymLen = wcslen(_stockSymBuff);

	int i = 0;
	while (i < stockSymLen && t < _sendBuff + WorkerThread::stockSymBuffLen - a - b - 1)
	{
		// check if the UI thread wants to end the worker thread.
		if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
			return;

		// we only use digits and letters for stock symbols.
		if ((_stockSymBuff[i] >= L'0' && _stockSymBuff[i <= L'9']) ||
			(_stockSymBuff[i] >= L'a' && _stockSymBuff[i <= L'z']) ||
			(_stockSymBuff[i] >= L'A' && _stockSymBuff[i <= L'Z']))
		{
			*t = '0' + _stockSymBuff[i] - L'0';
			t++;
		}
		else
		{
			if (*t != '+')
			{
				*t = '+';
				t++;
			}
		}

		i ++;
	}

	memcpy(t /*dest*/, WorkerThread::requestEnd /*src*/, b /*num*/);

	t += b;

	* (t+1) = '\0';
}

//
// Initializes WSA layer and addrinfo
BOOL WorkerThread::InitializeInternetConnection()
{

	// Initialize WSA layer (the WinSock dll) with the 2.2 version.
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
	{
		_wsaInitializedSucc = FALSE;
		return FALSE;
	}
	_wsaInitializedSucc = TRUE;

	// Initialize _addrInfo struct
	struct addrinfo hints;
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo(WorkerThread::HOST, WorkerThread::DEFAULT_PORT, &hints, &_addrInfo) != 0)
	{
		_addrInitializedSucc = FALSE;
		return FALSE;
	}
	_addrInitializedSucc = TRUE;

	return TRUE;
}

//
// Initializes the Socket to connect to the Internet.
// It is its own function because, if we have dropped connections, we will connect
// again and again.
//
void WorkerThread::InitializeSocket()
{
	if (_socketConnected)
		return;

	struct addrinfo * ptr = NULL;

	for(ptr=_addrInfo; ptr != NULL ;ptr=ptr->ai_next)
	{

		// Create a SOCKET for connecting to server
		_connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (_connectSocket == INVALID_SOCKET)
		{
			return;
		}

		// Connect to server.
		if (connect( _connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(_connectSocket);
			_connectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	_socketConnected = (_connectSocket != INVALID_SOCKET);
}

//
// The function that does all the work.
// This gets the data from the internet.
// If there is an invalid socket (but we have a valid addrinfo and WSA was initialized)
// 		the function will try to recreate the socket.
//
void WorkerThread::GetData()
{
	memset (_recvBuff, 0, sizeof(_recvBuff));

	if (!_wsaInitializedSucc || !_addrInitializedSucc)
	{
		// Oops - Winsock is dead in the water or DNS cannot return us an IP address.
		// not much we can do here.
		memcpy(_recvBuff /*dest*/, WorkerThread::CannotConnectToInternet /*src*/, strlen(WorkerThread::CannotConnectToInternet)/*num*/);
		return;
	}

	// Before making calls to the Internet, check if the UI thread is waiting on us to end
	if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
		return;

	if (!_socketConnected)
	{
		InitializeSocket();
	}

	if (!_socketConnected)
	{
		// Ok, we did not connect this time.
		// Put an error message and we are done.
		memcpy(_recvBuff /*dest*/, WorkerThread::ConnectionDropped /*src*/, strlen(WorkerThread::ConnectionDropped)/*num*/);
		return;
	}

	// Life is good!

	// Before making calls to the Internet, check if the UI thread is waiting on us to end
	if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
		return;

    if (send(_connectSocket, _sendBuff, (int)strlen(_sendBuff), 0 ) == SOCKET_ERROR)
    {
    	// Request did not go through. Will try again later.
        closesocket(_connectSocket);
        _socketConnected = FALSE;
        _connectSocket	 = INVALID_SOCKET;
		memcpy(_recvBuff /*dest*/, WorkerThread::InternetRequestDidNotWork /*src*/, strlen(WorkerThread::InternetRequestDidNotWork)/*num*/);
		return;
    }

	// Before making calls to the Internet, check if the UI thread is waiting on us to end
	if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
		return;

	recv(_connectSocket, _recvBuff, recvBuffLen, 0);

	// It looks like we have to close the socket because Yahoo is closing the connection on us quickly
	// I.e. before we get a chance to try again.
	closesocket(_connectSocket);
	_socketConnected = FALSE;
	_connectSocket = INVALID_SOCKET;
}


//
// takes the stock quotes from the internet and
// creates the result buffer to be displayed in the UI
//
void WorkerThread::CreateInternetResult()
{
	memset (_stockQuotesBuff, 0, sizeof(_stockQuotesBuff));

	char *t = _recvBuff;
	int i = 0;
	while (*t != '\0' && i < WorkerThread::stockQuotesBuffLen - 1)
	{
		// check if the UI thread wants to end the worker thread.
		if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
			return;

		if (*t == '\\')
		{
			// Yahoo uses the backslash to escape some characters.
			// Skip the backslash and the escaped character as well.
			t += 2;
			continue;
		};

		if (*t == '"')
		{
			t++;
			continue;
		}

		if (*t == ',' || *t == '\r' || *t == '\n')
			_stockQuotesBuff[i] = L' ';
		else
			_stockQuotesBuff[i] = L'0' + *t - '0';
		i++;
		t++;
	}

	_stockQuotesBuff[i] = L'\0';
}

//
// Cleans up the WinSock variables (WSA, addrInfo, Socket).
//
void WorkerThread::Cleanup()
{
	if (_addrInitializedSucc)
		freeaddrinfo(_addrInfo);

	// TODO: cleanup the socket
	if (_socketConnected)
	    closesocket(_connectSocket);

	if (_wsaInitializedSucc)
		WSACleanup();

	_wsaInitializedSucc = FALSE;
	_addrInitializedSucc = FALSE;
	_socketConnected = FALSE;
	_connectSocket = INVALID_SOCKET;
	_addrInfo = NULL;
}

//
// Init function.
// Called from the UI Thread.
// it sets the waitForWork and endThreadRequested events, a few state variables and it starts the working thread.
//
FUNC_RESULT WorkerThread::Init()
{
	_waitForWorkEvent = CreateEvent(NULL /*lpEventAttributes*/, TRUE /*fManualReset*/, FALSE /*bInitialState*/, NULL /*lpName*/);
	_endThreadRequestedEvent = CreateEvent(NULL /*lpEventAttributes*/, TRUE /*fManualReset*/, FALSE /*bInitialState*/, NULL /*lpName*/);

	_UIRequestedEndThread = WorkerThread::endThreadNotRequested;

	_wsaInitializedSucc = FALSE;
	_addrInitializedSucc = FALSE;
	_socketConnected = FALSE;
	_connectSocket = INVALID_SOCKET;
	_addrInfo = NULL;

	_beginthread(WorkerThread::Thread, 0, this);

	return SUCCESS;
}

void WorkerThread::Thread(PVOID pvoid)
{
	WorkerThread *wt = reinterpret_cast <WorkerThread *> (pvoid);

	// Initialize the connection to the Internet.
	// If the initialization fails we will just post a message to the UI thread that says "connection not available" or something like that.
	wt->InitializeInternetConnection();

	// work
	wt->WorkLoop();

	// cleanup after all that work.
	wt->Cleanup();

	// We exited the work loop means that we were signaled by the UI thread.
	// And that means that the UI thread is waiting on the _endThreadRequestedEvent.
	// Signal the event and we are done
	SetEvent(wt->_endThreadRequestedEvent);

	_endthread();
}

//
// The work horse.
//
void WorkerThread::WorkLoop()
{
	while (true)
	{
		// block waiting for work
		WaitForSingleObject(_waitForWorkEvent, INFINITE);

		// check if the UI thread wants to end the worker thread.
		if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
			break;

		// now convert the stock symbols into something that Yahoo Finance will use
		CreateInternetRequest();

		if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
			break;

		// send the request to the internet and wait for the result to come back
		GetData();

		if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
			break;

		// convert the result into a format to be displayed in the UI
		CreateInternetResult();

		if (_UIRequestedEndThread == WorkerThread::endThreadRequested)
			break;

		// we did our work, reset the event
		ResetEvent(_waitForWorkEvent);

		// Now do it again.
	}
}

WorkerThread::~WorkerThread()
{
}


