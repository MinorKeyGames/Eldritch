#include <WinSock2.h>
#include <WS2tcpip.h>

#include "core.h"
#include "httpsocket.h"
#include "configmanager.h"
#include "thread.h"

#include <memory.h>

DWORD WINAPI ConnectThreadProc( void* Parameter )
{
	HTTPSocket* pSocket = static_cast< HTTPSocket* >( Parameter );
	ASSERT( pSocket );

	if( connect( pSocket->m_Socket, ( SOCKADDR* )&pSocket->m_AsyncSocketAddress, sizeof( SOCKADDR ) ) == SOCKET_ERROR )
	{
		pSocket->m_AsyncConnected = false;
	}
	else
	{
		pSocket->m_AsyncConnected = true;
	}

	return 0;
}

DWORD WINAPI SendAndReceiveThreadProc( void* Parameter )
{
	HTTPSocket* pSocket = static_cast< HTTPSocket* >( Parameter );
	ASSERT( pSocket );

	uint BytesSent = send( pSocket->m_Socket, pSocket->m_AsyncSendBuffer.GetData(), pSocket->m_AsyncSendBuffer.Size(), 0 );
	Unused( BytesSent );
	ASSERT( BytesSent == pSocket->m_AsyncSendBuffer.Size() );

	{
		uint BytesReceived = 0;
		uint TotalBytesReceived = 0;

		do
		{
			BytesReceived = recv( pSocket->m_Socket, pSocket->m_AsyncReceiveBuffer.GetData() + TotalBytesReceived, pSocket->m_AsyncReceiveBufferSize, 0 );
			TotalBytesReceived += BytesReceived;

			// Treat errors as end of stream
			if( BytesReceived == SOCKET_ERROR )
			{
				BytesReceived = 0;
			}
		}
		while( BytesReceived );

		pSocket->m_AsyncReceiveBuffer[ TotalBytesReceived ] = '\0';
		pSocket->m_AsyncBytesReceived = TotalBytesReceived;
	}

	return 0;
}

HTTPSocket::SSocketInit::SSocketInit()
:	m_HostName( "" )
,	m_IPAddress( "" )
,	m_Path( "" )
,	m_ContentType( "" )
,	m_CloseConnection( false )
,	m_Async( false )
{
}

HTTPSocket::HTTPSocket()
:	m_SocketInit()
,	m_Socket( NULL )
,	m_Thread( NULL )
,	m_AsyncConnected( false )
,	m_AsyncSocketAddress()
,	m_AsyncSendBuffer()
,	m_AsyncReceiveBuffer()
,	m_AsyncReceiveBufferSize( 0 )
,	m_AsyncBytesReceived( 0 )
{
	WSADATA SocketData;
	CHECK( NO_ERROR == WSAStartup( MAKEWORD( 2, 2 ), &SocketData ) );
}

HTTPSocket::~HTTPSocket()
{
	WSACleanup();

	SafeDelete( m_Thread );
}

bool HTTPSocket::Connect( const SSocketInit& SocketInit )
{
	m_SocketInit = SocketInit;
	ASSERT( m_SocketInit.m_HostName != "" || m_SocketInit.m_IPAddress != "" );

	m_Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( m_Socket == INVALID_SOCKET )
	{
		return false;
	}

	sockaddr_in SocketAddress;
	SocketAddress.sin_family = AF_INET;
	SocketAddress.sin_port = htons( 80 );	// HTTP is port 80

	if( m_SocketInit.m_HostName == "" )
	{
		// Use IP address
		SocketAddress.sin_addr.s_addr = inet_addr( m_SocketInit.m_IPAddress.CStr() );
	}
	else
	{
		addrinfo Hints;
		memset( &Hints, 0, sizeof( Hints ) );
		Hints.ai_family = AF_INET;
		Hints.ai_socktype = SOCK_STREAM;
		Hints.ai_protocol = IPPROTO_TCP;

		addrinfo* AddressInfo = NULL;
		int Success = getaddrinfo( m_SocketInit.m_HostName.CStr(), "80", &Hints, &AddressInfo );

		if( 0 == Success && AddressInfo )
		{
			sockaddr_in* ResolvedSocketAddress = ( sockaddr_in* )AddressInfo->ai_addr;

			// Use IP address resolved from host name
			SocketAddress.sin_addr.s_addr = ResolvedSocketAddress->sin_addr.s_addr;
		}

		if( AddressInfo )
		{
			freeaddrinfo( AddressInfo );
		}
	}

	if( m_SocketInit.m_Async )
	{
		m_AsyncSocketAddress = SocketAddress;
		SafeDelete( m_Thread );
		m_Thread = new Thread( ConnectThreadProc, this );

		return true;
	}
	else
	{
		// Blocking connect
		if( connect( m_Socket, ( SOCKADDR* )&SocketAddress, sizeof( SocketAddress ) ) == SOCKET_ERROR )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

void HTTPSocket::Get( Array< char >& OutBuffer, const SimpleString& AgentName )
{
	SimpleString Header = "";
	Header +=
		SimpleString( "GET " ) + m_SocketInit.m_Path + SimpleString( " HTTP/1.1" CRLF ) +
		SimpleString( "Host: " ) + m_SocketInit.m_HostName + SimpleString( CRLF ) +
		SimpleString( "User-Agent: " ) + AgentName + SimpleString( CRLF );

	if( m_SocketInit.m_CloseConnection )
	{
		Header += SimpleString( "Connection: close" CRLF );
	}

	Header += SimpleString( CRLF );

	Array< char > RequestBuffer;
	uint RequestBufferSize = Header.Length() + 1;
	RequestBuffer.Resize( RequestBufferSize );
	memcpy_s( RequestBuffer.GetData(), RequestBufferSize, Header.CStr(), RequestBufferSize );

	SendAndReceive( RequestBuffer, OutBuffer );
}

void HTTPSocket::Post( const Array< char >& Buffer, const SimpleString& AgentName, Array< char >& OutBuffer )
{
	uint BufferSize = Buffer.Size();

	SimpleString Header = "";
	Header +=
		SimpleString( "POST " ) + m_SocketInit.m_Path + SimpleString( " HTTP/1.1" CRLF ) +
		SimpleString( "Host: " ) + m_SocketInit.m_HostName + SimpleString( CRLF ) +
		SimpleString( "Content-Type: " ) + m_SocketInit.m_ContentType + SimpleString( CRLF ) +
		SimpleString( "Content-Length: " ) + SimpleString::PrintF( "%d", BufferSize ) + SimpleString( CRLF ) +
		SimpleString( "User-Agent: " ) + AgentName + SimpleString( CRLF );

	if( m_SocketInit.m_CloseConnection )
	{
		Header += SimpleString( "Connection: close" CRLF );
	}

	Header += SimpleString( CRLF );

	Array< char > RequestBuffer;
	uint RequestBufferSize = Header.Length();	// Don't include the terminating null; we're appending to the header here
	RequestBuffer.Resize( RequestBufferSize + BufferSize );
	memcpy_s( RequestBuffer.GetData(), RequestBufferSize, Header.CStr(), RequestBufferSize );
	memcpy_s( RequestBuffer.GetData() + RequestBufferSize, BufferSize, Buffer.GetData(), BufferSize );

	RequestBuffer.PushBack( '\0' );

	SendAndReceive( RequestBuffer, OutBuffer );
}

void HTTPSocket::Post( const SimpleString& Buffer, const SimpleString& AgentName, Array< char >& OutBuffer )
{
	uint BufferSize = Buffer.Length();

	SimpleString Header = "";
	Header +=
		SimpleString( "POST " ) + m_SocketInit.m_Path + SimpleString( " HTTP/1.1" CRLF ) +
		SimpleString( "Host: " ) + m_SocketInit.m_HostName + SimpleString( CRLF ) +
		SimpleString( "Content-Type: " ) + m_SocketInit.m_ContentType + SimpleString( CRLF ) +
		SimpleString( "Content-Length: " ) + SimpleString::PrintF( "%d", BufferSize ) + SimpleString( CRLF ) +
		SimpleString( "User-Agent: " ) + AgentName + SimpleString( CRLF );

	if( m_SocketInit.m_CloseConnection )
	{
		Header += SimpleString( "Connection: close" CRLF );
	}

	Header += SimpleString( CRLF );

	Array< char > RequestBuffer;
	uint RequestBufferSize = Header.Length();	// Don't include the terminating null; we're appending to the header here
	RequestBuffer.Resize( RequestBufferSize + BufferSize );
	memcpy_s( RequestBuffer.GetData(), RequestBufferSize, Header.CStr(), RequestBufferSize );
	memcpy_s( RequestBuffer.GetData() + RequestBufferSize, BufferSize, Buffer.CStr(), BufferSize );

	RequestBuffer.PushBack( '\0' );

	SendAndReceive( RequestBuffer, OutBuffer );
}

void HTTPSocket::SendAndReceive( const Array< char >& Buffer, Array< char >& OutBuffer )
{
#if BUILD_DEV
	// Assert that we can send a message of the given size all at once
	uint MaxSize;
	int SizeOf = sizeof( uint );
	if( getsockopt( m_Socket, SOL_SOCKET, SO_MAX_MSG_SIZE, ( char* )&MaxSize, &SizeOf ) != SOCKET_ERROR )
	{
		ASSERT( MaxSize >= Buffer.Size() );
	}
#endif

	if( m_SocketInit.m_Async )
	{
		m_AsyncSendBuffer = Buffer;

		SafeDelete( m_Thread );
		m_Thread = new Thread( SendAndReceiveThreadProc, this );
	}
	else
	{
		// Blocking send
		uint BytesSent = send( m_Socket, Buffer.GetData(), Buffer.Size(), 0 );
		Unused( BytesSent );
		ASSERT( BytesSent == Buffer.Size() );

		// Blocking receive
		STATICHASH( HTTPSocketTempBufferSize );
		int TempBufferSize = ConfigManager::GetInt( sHTTPSocketTempBufferSize, 1024 );
		uint BytesReceived = 0;
		char* TempBuffer = new char[ TempBufferSize ];

		OutBuffer.Clear();

		do
		{
			BytesReceived = recv( m_Socket, TempBuffer, TempBufferSize, 0 );

			// Treat errors as end of stream
			if( BytesReceived == SOCKET_ERROR )
			{
				BytesReceived = 0;
			}

			// Copy temp buffered data into out buffer
			uint BufferSize = OutBuffer.Size();
			OutBuffer.Resize( BufferSize + BytesReceived );
			memcpy_s( OutBuffer.GetData() + BufferSize, BytesReceived, TempBuffer, BytesReceived );
		}
		while( BytesReceived );

		SafeDelete( TempBuffer );

		OutBuffer.PushBack( '\0' );
	}
}

bool HTTPSocket::AsyncIsWaiting() const
{
	return m_Thread && !m_Thread->IsDone();
}

bool HTTPSocket::AsyncConnectSucceeded() const
{
	return m_AsyncConnected;
}

const Array< char >& HTTPSocket::AsyncGetReceived() const
{
	return m_AsyncReceiveBuffer;
}

void HTTPSocket::AsyncSetReceiveBufferSize( uint NewSize )
{
	m_AsyncReceiveBufferSize = NewSize;

	ASSERT( m_AsyncReceiveBufferSize > 0 );
	m_AsyncReceiveBuffer.Reserve( m_AsyncReceiveBufferSize );
	m_AsyncReceiveBuffer.Resize( m_AsyncReceiveBufferSize );
}

uint HTTPSocket::AsyncGetBytesReceived() const
{
	return m_AsyncBytesReceived;
}