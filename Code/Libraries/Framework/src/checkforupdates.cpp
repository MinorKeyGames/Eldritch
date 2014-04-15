#include "core.h"
#include "checkforupdates.h"
#include "configmanager.h"
#include "Common/uimanagercommon.h"
#include "file.h"

#define SUPPRESS_AUTO_UPDATE	( 1 && BUILD_DEV )
#define FORCE_SHOULD_UPDATE		0

CheckForUpdates::CheckForUpdates( UIManagerCommon* pUIManager )
:	m_UIManager( pUIManager )
,	m_Socket()
,	m_ResultBuffer()
,	m_WaitingToConnect( false )
,	m_WaitingToReceive( false )
,	m_Verbose( false )
,	m_PausesGame( false )
{
	STATICHASH( HTTPSocketAsyncReceiveBufferSize );

	const int BufferSize = ConfigManager::GetInt( sHTTPSocketAsyncReceiveBufferSize, 512 * 1024 );

	m_Socket.AsyncSetReceiveBufferSize( BufferSize );

	m_ResultBuffer.Reserve( BufferSize );
	m_ResultBuffer.SetDeflate( false );
}

CheckForUpdates::~CheckForUpdates()
{
}

void CheckForUpdates::Tick()
{
	XTRACE_FUNCTION;

	STATICHASH( CheckForUpdatesAgent );

	if( m_WaitingToConnect && IsRequestFinished() )
	{
		if( m_Socket.AsyncConnectSucceeded() )
		{
			m_Socket.Get( m_ResultBuffer, ConfigManager::GetString( sCheckForUpdatesAgent ) );

			m_WaitingToConnect = false;
			m_WaitingToReceive = true;
		}
		else
		{
			m_WaitingToConnect = false;
			m_WaitingToReceive = false;

			if( m_Verbose )
			{
				m_UIManager->HideWaitDialog();
				m_UIManager->ShowOKDialog( m_PausesGame, "CouldNotConnect", "", "Pop", "" );
			}
		}
	}
	else if( m_WaitingToReceive && IsRequestFinished() )
	{
		m_ResultBuffer = m_Socket.AsyncGetReceived();
		m_WaitingToReceive = false;

		OnReceive();
	}
}

void CheckForUpdates::Check( bool Verbose, bool PausesGame )
{
#if SUPPRESS_AUTO_UPDATE
	// Suppress auto updates for dev builds
	if( !Verbose )
	{
		PRINTF( "Suppressing auto update.\n" );
		return;
	}
#endif

	m_PausesGame = PausesGame;

	if( m_WaitingToConnect || m_WaitingToReceive )
	{
		// If user manually checks for updates while we're already checking, elevate to verbose but don't restart connection
		if( Verbose && !m_Verbose )
		{
			m_Verbose = true;
			m_UIManager->ShowWaitDialog( m_PausesGame, "Connecting", "" );
		}

		return;
	}

	m_Verbose = Verbose;
	if( m_Verbose )
	{
		m_UIManager->ShowWaitDialog( m_PausesGame, "Connecting", "" );
	}

	m_WaitingToConnect = true;
	m_WaitingToReceive = false;

	HTTPSocket::SSocketInit SocketInit;
	InitSocket( SocketInit );
	m_Socket.Connect( SocketInit );
}

bool CheckForUpdates::IsRequestFinished()
{
	return !m_Socket.AsyncIsWaiting();
}

void CheckForUpdates::InitSocket( HTTPSocket::SSocketInit& SocketInit )
{
	STATICHASH( ContentSyncer );
	STATICHASH( PatcherHost );
	STATICHASH( PatcherVersionPath );

	SocketInit.m_CloseConnection = true;
	SocketInit.m_HostName = ConfigManager::GetString( sPatcherHost, "", sContentSyncer );
	SocketInit.m_Path = ConfigManager::GetString( sPatcherVersionPath, "", sContentSyncer );
	SocketInit.m_Async = true;
}

// Based on Patcher_HandleReceipt_VersionNumber in ContentSyncer
void CheckForUpdates::OnReceive()
{
	char* MessageContent = strstr( m_ResultBuffer.GetData(), CRLF CRLF );
	if( !MessageContent )
	{
		if( m_Verbose )
		{
			m_UIManager->HideWaitDialog();
			m_UIManager->ShowOKDialog( m_PausesGame, "CouldNotReceive", "", "Pop", "" );
		}

		return;
	}

	MessageContent += 4;	// Skip the CRLF CRLF

	// If there's additional content, only get the first line
	char* LineBreak = strstr( MessageContent, LF );
	if( LineBreak )
	{
		*LineBreak = '\0';
	}

	SimpleString LatestVersion = MessageContent;

	STATICHASH( Version );
	STATICHASH( ContentSyncer );
	SimpleString LocalVersion = ConfigManager::GetString( sVersion, "", sContentSyncer );

	if( m_Verbose )
	{
		m_UIManager->HideWaitDialog();
	}

#if !FORCE_SHOULD_UPDATE
	const int iLocalVersion		= LocalVersion.AsInt();
	const int iLatestVersion	= LatestVersion.AsInt();

	if( iLocalVersion >= iLatestVersion )
	{
		if( m_Verbose )
		{
			m_UIManager->ShowOKDialog( m_PausesGame, "UpToDate", "", "Pop", "" );
		}
	}
	else
#endif
	{
		m_UIManager->ShowYesNoDialog( m_PausesGame, "ShouldUpdate", "", "", "Pop", "", "", SUICallback( TryRunContentSyncer, this ) );
	}
}

/*static*/ void CheckForUpdates::TryRunContentSyncer( void* pUIElement, void* pVoid )
{
	Unused( pUIElement );

	if( FileUtil::Launch( "ContentSyncer.exe" ) )
	{
		PostQuitMessage( 0 );
	}
	else if( pVoid )
	{
		CheckForUpdates* pCheckForUpdates = static_cast<CheckForUpdates*>( pVoid );
		pCheckForUpdates->m_UIManager->ShowOKDialog( pCheckForUpdates->m_PausesGame, "CouldNotRunContentSyncer", "", "Pop", "" );
	}
}