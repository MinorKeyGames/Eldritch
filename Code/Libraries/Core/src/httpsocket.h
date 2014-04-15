#ifndef HTTPSOCKET_H
#define HTTPSOCKET_H

#include "simplestring.h"
#include "array.h"

#include <Windows.h>

class Thread;
struct sockaddr_in;
typedef __w64 uint SOCKET;

#define CRLF "\r\n"
#define LF "\n"

class HTTPSocket
{
public:
	struct SSocketInit
	{
		SSocketInit();

		SimpleString	m_HostName;
		SimpleString	m_IPAddress;	// Only needed if HostName is not provided, and it should be for doing the HTTP requests anyway
		SimpleString	m_Path;
		SimpleString	m_ContentType;
		bool			m_CloseConnection;
		bool			m_Async;
	};

	HTTPSocket();
	~HTTPSocket();

	bool	Connect( const SSocketInit& SocketInit );
	void	Get( Array< char >& OutBuffer, const SimpleString& AgentName );
	void	Post( const Array< char >& Buffer, const SimpleString& AgentName, Array< char >& OutBuffer );	// Don't use for posting strings, as it will post the terminating null
	void	Post( const SimpleString& Buffer, const SimpleString& AgentName, Array< char >& OutBuffer );

	void					AsyncSetReceiveBufferSize( uint NewSize );
	bool					AsyncIsWaiting() const;
	bool					AsyncConnectSucceeded() const;
	const Array< char >&	AsyncGetReceived() const;
	uint					AsyncGetBytesReceived() const;

private:
	void	SendAndReceive( const Array< char >& Buffer, Array< char >& OutBuffer );

	SSocketInit	m_SocketInit;
	SOCKET		m_Socket;

	// Async members
	Thread*			m_Thread;
	bool			m_AsyncConnected;
	sockaddr_in		m_AsyncSocketAddress;
	Array< char >	m_AsyncSendBuffer;
	Array< char >	m_AsyncReceiveBuffer;
	uint			m_AsyncReceiveBufferSize;
	uint			m_AsyncBytesReceived;

	friend DWORD WINAPI ConnectThreadProc( void* Parameter );
	friend DWORD WINAPI SendAndReceiveThreadProc( void* Parameter );
};

#endif // HTTPSOCKET_H