#include "core.h"
#include "asciirenderer.h"
#include "asciisurface.h"
#include "clock.h"
#include "simplestring.h"
#include "filestream.h"
#include "packstream.h"
#include "configmanager.h"
#include "stringmanager.h"
#include "array.h"
#include "file.h"
#include "mathcore.h"
#include "httpsocket.h"
#include "datapipe.h"
#include "checksum.h"
#include "keyboard.h"
#include "allocator.h"

#include <Windows.h>
#include <crtdbg.h>

Clock*			g_Clock = NULL;

ASCIIRenderer*	g_Renderer = NULL;
ASCIISurface*	g_MenuSurface = NULL;
ASCIISurface*	g_BackBufferSurface = NULL;

Keyboard*		g_Keyboard = NULL;

bool			g_ShouldExit = false;
bool			g_ShouldRestart = false;
bool			g_CheckForRelaunch = true;
bool			g_ShouldLaunchApp = false;

HTTPSocket*		g_HTTPSocket = NULL;
uint			g_HTTPBufferSize = 0;

Array< char >	g_HTTPResultBuffer;

Array< SimpleString >	g_CommandLineArguments;

struct SManifestFile
{
	SimpleString	m_Filename;
	SimpleString	m_Length;
	SimpleString	m_Checksum;
	bool			m_Validated;
};

Array< SManifestFile >	g_FilesInManifest;
uint					g_NextPatchFileIndex = 0;

bool			g_Patcher_GettingVersionNumber = false;
bool			g_Patcher_GettingManifestFile = false;
bool			g_Patcher_GettingNextPatchFile = false;

bool			g_Patcher_WaitingToConnect = false;
bool			g_Patcher_WaitingToReceive = false;

bool			g_Patcher_IsUpdating = false;

uint			g_MainMemorySize = 128 * 1024 * 1024;	// Make sure this is big enough for the buffer defined in the syncer config, and then some.

uint			g_NumWarnings = 0;

struct SStatus
{
	uint8			m_Color;
	SimpleString	m_Status;
};
Array<SStatus>		g_Statuses;
uint				g_StatusOffset = 0;	// Which one we're currently looking at

struct SCommandLine
{
	SimpleString		m_CommandLine;
	SimpleString		m_App;
	Array<SimpleString>	m_Arguments;
};

SCommandLine*	g_CommandLine = NULL;

uint8			g_StatusColor = FG_WHITE;
uint8			g_StatusWarningColor = FG_BR_RED;

uint			g_ConsoleWidth = 80;
uint			g_ConsoleHeight = 24;

uint			g_NumStatusLines = g_ConsoleHeight;

#if BUILD_DEV
uint			g_DEBUGTickSimCount = 0;
uint			g_DEBUGTickRenderCount = 0;
#endif

void Main( int argc, char* argv[] );

void SaveCommandLineArguments( int argc, char* argv[] );
bool HasCommandLineArgument( const SimpleString& Argument );

void Initialize();
void ShutDown();

bool Tick();
bool TickSim();
void TickRender();

bool Launch( const SimpleString& CommandLine );
bool LaunchApp();
bool RelaunchTempSelf();
bool RelaunchTrueSelf();

void InitializeConfigFiles();
void ResolveCommandLine();
void RedrawScene();
void AddStatus( const SimpleString& Status, uint8 Color );
void CopyTempPathFiles();
SimpleString GetTempFileForPatchFile( const SimpleString& PatchFile );
void BeginUpdating();
void Reinit();
void SplitIntoLines( const char* Message,uint MessageLength, Array<SimpleString>& OutLines );
void FinishPatching();
bool CheckForRelaunch();
void TryLaunchApp();

bool Patcher_IsRequestFinished();
void Patcher_Tick();
void Patcher_HandleReceipt();
void Patcher_HandleReceipt_VersionNumber();
void Patcher_HandleReceipt_ManifestFile();
void Patcher_HandleReceipt_NextPatchFile();
void Patcher_ReportFailure();
void Patcher_GetVersionNumber();
void Patcher_GetManifestFile();
void Patcher_GetNextPatchFile();

int main( int argc, char* argv[] )
{
	Allocator::Enable( true );
	Allocator::GetDefault().Initialize( g_MainMemorySize );

	Main( argc, argv );

#if DO_PROFILING
#if BUILD_DEBUG	// I ship Release ContentSyncer, not Final.
	Profiler::GetInstance()->Dump( FileStream( "profiler.txt", FileStream::EFM_Write ) );
#endif
	Profiler::DeleteInstance();
#endif

#if BUILD_DEBUG
	Allocator::GetDefault().Report( FileStream( "content-syncer-memory-exit-report.txt", FileStream::EFM_Write ) );
#endif
	DEBUGASSERT( Allocator::GetDefault().CheckForLeaks() );
	Allocator::GetDefault().ShutDown();

	DEBUGASSERT( _CrtCheckMemory() );
	DEBUGASSERT( !_CrtDumpMemoryLeaks() );

	return 0;
}

void Main( int argc, char* argv[] )
{
	SaveCommandLineArguments( argc, argv );

	Initialize();
	while( Tick() );
	ShutDown();
}

void SaveCommandLineArguments( int argc, char* argv[] )
{
	for( int ArgumentIndex = 0; ArgumentIndex < argc; ++ArgumentIndex )
	{
		g_CommandLineArguments.PushBack( SimpleString( argv[ ArgumentIndex ] ) );
	}
}

bool HasCommandLineArgument( const SimpleString& Argument )
{
	for( uint ArgumentIndex = 0; ArgumentIndex < g_CommandLineArguments.Size(); ++ArgumentIndex )
	{
		if( g_CommandLineArguments[ ArgumentIndex ].StrICmp( Argument ) )
		{
			return true;
		}
	}

	return false;
}

void Initialize()
{
	STATICHASH( ContentSyncer );
	STATICHASH( Application );
	STATICHASH( Title );
	STATICHASH( OnlyRedrawDirtyCells );
	STATICHASH( HTTPBufferSize );

	// Don't print to console, because it will mess with game visuals.
	SETPRINTCHANNELS( PRINTCHANNEL_Output | PRINTCHANNEL_Log );

	InitializeConfigFiles();

	g_Clock				= new Clock;

	g_Keyboard			= new Keyboard;

	g_Renderer			= new ASCIIRenderer;
	g_MenuSurface		= new ASCIISurface;
	g_BackBufferSurface	= new ASCIISurface;

	g_Renderer->Init();
	g_Renderer->SetFlickerFreeMode( ConfigManager::GetBool( sOnlyRedrawDirtyCells, false, sContentSyncer ) );

	SetConsoleTitle( ConfigManager::GetString( sTitle, "", sContentSyncer ) );

	g_CommandLine		= new SCommandLine;
	g_CommandLine->m_App = ConfigManager::GetString( sApplication, "", sContentSyncer );
	ResolveCommandLine();

	g_HTTPBufferSize = ConfigManager::GetInt( sHTTPBufferSize, 0, sContentSyncer );

	g_HTTPSocket		= new HTTPSocket;
	g_HTTPSocket->AsyncSetReceiveBufferSize( g_HTTPBufferSize );

	AddStatus( SimpleString::PrintF( "%s initialized.", ConfigManager::GetString( sTitle, "", sContentSyncer ) ), g_StatusColor );

	Patcher_GetVersionNumber();
}

void ShutDown()
{
#if BUILD_DEV
	PRINTF( "SimCount: %d RenderCount: %d\n", g_DEBUGTickSimCount, g_DEBUGTickRenderCount );
#endif

	g_Statuses.Clear();
	g_HTTPResultBuffer.Clear();
	g_FilesInManifest.Clear();
	g_CommandLineArguments.Clear();

	SafeDelete( g_CommandLine );

	SafeDelete( g_Clock );

	SafeDelete( g_Keyboard );

	SafeDelete( g_Renderer );
	SafeDelete( g_MenuSurface );
	SafeDelete( g_BackBufferSurface );

	SafeDelete( g_HTTPSocket );

	StringManager::DeleteInstance();
	PrintManager::DeleteInstance();
	ConfigManager::DeleteInstance();
	PackStream::StaticShutDown();
}

bool Tick()
{
	bool DoNextTick = true;

	DoNextTick = TickSim();

	TickRender();

	SwitchToThread();

	return DoNextTick;
}

bool TickSim()
{
	PROFILE_FUNCTION;

#if BUILD_DEV
	++g_DEBUGTickSimCount;
#endif

	if( g_CheckForRelaunch )
	{
		g_CheckForRelaunch = false;
		if( CheckForRelaunch() )
		{
			return false;
		}
	}

	if( g_ShouldRestart )
	{
		g_ShouldRestart = false;
		if( RelaunchTempSelf() )
		{
			return false;
		}
	}

	if( g_ShouldLaunchApp )
	{
		TryLaunchApp();
	}

	Patcher_Tick();

	return !g_ShouldExit;
}

void TickRender()
{
	PROFILE_FUNCTION;

#if BUILD_DEV
	++g_DEBUGTickRenderCount;
#endif

	RedrawScene();

	ASCIISurface::ABlit( *g_MenuSurface, *g_BackBufferSurface );
	g_Renderer->Draw( *g_BackBufferSurface );
}

bool Launch( SimpleString& CommandLine )
{
	if( g_NumWarnings > 0 )
	{
		AddStatus( SimpleString::PrintF( "\nContentSyncer finished with %d warnings.\n\nPress Enter to continue.", g_NumWarnings ), g_StatusColor );

		// Mini loop in here, ugh!
		for(;;)
		{
			g_Keyboard->Tick( 0.0f );

			if( GetForegroundWindow() == GetConsoleWindow() )
			{
				if( g_Keyboard->OnRise( Keyboard::EB_Enter ) )
				{
					break;
				}
			}

			TickRender();
			SwitchToThread();
		}
	}

	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;

	ZeroMemory( &StartupInfo, sizeof( StartupInfo ) );
	StartupInfo.cb = sizeof( StartupInfo );

	ZeroMemory( &ProcessInfo, sizeof( ProcessInfo ) );

	// See Process Creation Flags
	DWORD Flags =
		CREATE_DEFAULT_ERROR_MODE |		// Don't inherit error mode
		CREATE_NEW_CONSOLE;				// Create new console (DETACHED_PROCESS also means this console isn't inherited; but a new console must be created with AllocConsole)

	BOOL Result = CreateProcess(
		NULL,						// Application name (optional--use the first argument from command line if NULL)
		CommandLine.MutableCStr(),	// Command line (optional, includes application name)
		NULL,						// Process attributes (optional pointer to a SECURITY_ATTRIBUTES)
		NULL,						// Thread attributes (optional pointer to a SECURITY_ATTRIBUTES)
		FALSE,						// Inherit handles
		Flags,						// Creation flags (see Process Creation Flags)
		NULL,						// Environment block (optional void*)
		NULL,						// Current directory (optional string)
		&StartupInfo,				// Startup info
		&ProcessInfo				// Process info
		);

	return Result == TRUE;
}

bool LaunchApp()
{
	return Launch( g_CommandLine->m_CommandLine );
}

bool RelaunchTempSelf()
{
	STATICHASH( ContentSyncer );
	STATICHASH( TempSelf );
	STATICHASH( RelaunchArg );

	SimpleString CommandLine = SimpleString::PrintF( "%s %s",
		ConfigManager::GetString( sTempSelf, "", sContentSyncer ),
		ConfigManager::GetString( sRelaunchArg, "", sContentSyncer ) );

	return Launch( CommandLine );
}

bool RelaunchTrueSelf()
{
	STATICHASH( ContentSyncer );
	STATICHASH( Self );

	SimpleString CommandLine = ConfigManager::GetString( sSelf, "", sContentSyncer );

	return Launch( CommandLine );
}

void ResolveCommandLine()
{
	g_CommandLine->m_CommandLine = g_CommandLine->m_App;
}

void RedrawScene()
{
	g_MenuSurface->Fill( ' ', BG_BLACK );

	if( g_NumStatusLines > 0 )
	{
		uint NumStatusesToDraw = Min( g_NumStatusLines, g_Statuses.Size() - g_StatusOffset );
		for( uint LineIndex = 0; LineIndex < NumStatusesToDraw; ++LineIndex )
		{
			uint StatusIndex = LineIndex + g_StatusOffset;

			SStatus Status = g_Statuses[ StatusIndex ];
			g_MenuSurface->Print( Status.m_Status.CStr(), g_ConsoleWidth, Status.m_Color, 0, LineIndex, ASURF_FG );
		}
	}
}

void AddStatus( const SimpleString& Status, uint8 Color )
{
	SStatus NewStatus;
	NewStatus.m_Status = Status;
	NewStatus.m_Color = Color;

	g_Statuses.PushBack( NewStatus );
	g_StatusOffset = g_Statuses.Size() > g_NumStatusLines ? g_Statuses.Size() - g_NumStatusLines : 0;

	PRINTF( SimpleString::PrintF( "Status: %s\n", Status.CStr() ).CStr() );
}

void TryLaunchApp()
{
	if( LaunchApp() )
	{
		g_ShouldExit = true;
	}
	else
	{
		AddStatus( SimpleString::PrintF( "Failed to launch %s", g_CommandLine->m_App.CStr() ), g_StatusWarningColor );
		++g_NumWarnings;
	}
}

bool Patcher_IsRequestFinished()
{
	ASSERT( g_HTTPSocket );
	return !g_HTTPSocket->AsyncIsWaiting();
}

void Patcher_Tick()
{
	STATICHASH( PatcherAgent );
	STATICHASH( ContentSyncer );

	if( g_Patcher_WaitingToConnect && Patcher_IsRequestFinished() )
	{
		if( g_HTTPSocket->AsyncConnectSucceeded() )
		{
			// NOTE: For async socket, buffer isn't actually filled here. Just a bad interface design.
			g_HTTPSocket->Get( g_HTTPResultBuffer, ConfigManager::GetString( sPatcherAgent, "", sContentSyncer ) );

			g_Patcher_WaitingToConnect = false;
			g_Patcher_WaitingToReceive = true;
		}
		else
		{
			// Connection failed

			g_Patcher_WaitingToConnect = false;
			ASSERT( !g_Patcher_WaitingToReceive );

			// If we were updating, we're not anymore
			g_Patcher_IsUpdating = false;

			Patcher_ReportFailure();
		}
	}
	else if( g_Patcher_WaitingToReceive && Patcher_IsRequestFinished() )
	{
		g_HTTPResultBuffer = g_HTTPSocket->AsyncGetReceived();

		ASSERT( !g_Patcher_WaitingToConnect );
		g_Patcher_WaitingToReceive = false;

		if( g_HTTPSocket->AsyncGetBytesReceived() )
		{
			Patcher_HandleReceipt();
		}
		else
		{
			// If we were updating, we're not anymore
			g_Patcher_IsUpdating = false;

			Patcher_ReportFailure();
		}
	}
}

void Patcher_HandleReceipt()
{
	if( g_Patcher_GettingVersionNumber )
	{
		Patcher_HandleReceipt_VersionNumber();
	}
	else if( g_Patcher_GettingManifestFile )
	{
		Patcher_HandleReceipt_ManifestFile();
	}
	else if( g_Patcher_GettingNextPatchFile )
	{
		Patcher_HandleReceipt_NextPatchFile();
	}
}

void Patcher_HandleReceipt_VersionNumber()
{
	char* MessageContent = strstr( g_HTTPResultBuffer.GetData(), CRLF CRLF );
	if( !MessageContent )
	{
		AddStatus( "Error receiving version number.", g_StatusWarningColor );
		++g_NumWarnings;
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

	if( LocalVersion == LatestVersion )
	{
		AddStatus( "Game is up to date.", g_StatusColor );
		g_ShouldLaunchApp = true;
	}
	else
	{
		AddStatus( "New updates are available.", g_StatusColor );
		BeginUpdating();
	}
}

void SplitIntoLines( const char* Message, uint MessageLength, Array<SimpleString>& OutLines )
{
	const char* LineStart = Message;
	const char* MessageEnd = Message + MessageLength;

	while( LineStart )
	{
		const char* LineEnd = LineStart;
		while( LineEnd < MessageEnd && *LineEnd != '\n' )
		{
			++LineEnd;
		}

		if( *LineEnd == '\n' )
		{
			OutLines.PushBack( SimpleString( LineStart, ( uint )( LineEnd - LineStart ) ) );
			LineStart = LineEnd + 1;
		}
		else
		{
			LineStart = NULL;
		}
	}
}

void Patcher_HandleReceipt_ManifestFile()
{
	g_FilesInManifest.Clear();
	g_NextPatchFileIndex = 0;

	char* MessageContent = strstr( g_HTTPResultBuffer.GetData(), CRLF CRLF );
	if( !MessageContent )
	{
		AddStatus( "Error receiving manifest file.", g_StatusWarningColor );
		++g_NumWarnings;
		return;
	}

	MessageContent += 4;

	uint Offset = ( uint )( MessageContent - g_HTTPResultBuffer.GetData() );
	uint ContentSize = g_HTTPSocket->AsyncGetBytesReceived() - Offset;

	Array<SimpleString>	Lines;
	SplitIntoLines( MessageContent, ContentSize, Lines );

	for( uint LinesIndex = 0; LinesIndex + 2 < Lines.Size(); LinesIndex += 3 )
	{
		SManifestFile ManifestFile;
		ManifestFile.m_Filename = Lines[ LinesIndex ];
		ManifestFile.m_Length = Lines[ LinesIndex + 1 ];
		ManifestFile.m_Checksum = Lines[ LinesIndex + 2 ];
		ManifestFile.m_Validated = false;

		g_FilesInManifest.PushBack( ManifestFile );
	}

	AddStatus( "Manifest file received.", g_StatusColor );

	if( g_NextPatchFileIndex < g_FilesInManifest.Size() )
	{
		Patcher_GetNextPatchFile();
	}
}

void Patcher_HandleReceipt_NextPatchFile()
{
	char* MessageContent = strstr( g_HTTPResultBuffer.GetData(), CRLF CRLF );
	if( !MessageContent )
	{
		// If we were updating, we're not anymore
		g_Patcher_IsUpdating = false;

		AddStatus( "Error receiving patch file.", g_StatusWarningColor );
		++g_NumWarnings;
		return;
	}

	MessageContent += 4;

	uint Offset = ( uint )( MessageContent - g_HTTPResultBuffer.GetData() );
	uint ContentSize = g_HTTPSocket->AsyncGetBytesReceived() - Offset;

	// Compute the checksum for the payload and compare with the data in the manifest file
	uint32 ContentChecksum = Checksum::Adler32( ( uint8* )MessageContent, ContentSize );
	SimpleString ContentLengthString = SimpleString::PrintF( "%d", ContentSize );
	SimpleString ContentChecksumString = SimpleString::PrintF( "0x%08X", ContentChecksum );

	SManifestFile& ManifestFile = g_FilesInManifest[ g_NextPatchFileIndex ];
	SimpleString PatchFile = ManifestFile.m_Filename;

	if( ContentLengthString == ManifestFile.m_Length && ContentChecksumString == ManifestFile.m_Checksum )
	{
		ManifestFile.m_Validated = true;

		FileStream TempFile( GetTempFileForPatchFile( PatchFile ).CStr(), FileStream::EFM_Write );
		TempFile.Write( ContentSize, MessageContent );

		AddStatus( SimpleString::PrintF( "%s received.", PatchFile.CStr() ), g_StatusColor );
	}
	else
	{
		AddStatus( SimpleString::PrintF( "Checksum failure for %s.", PatchFile.CStr() ), g_StatusWarningColor );
		++g_NumWarnings;
	}

	g_NextPatchFileIndex++;

	// If there are remaining files, get them.
	if( g_NextPatchFileIndex < g_FilesInManifest.Size() )
	{
		Patcher_GetNextPatchFile();
	}
	else
	{
		FinishPatching();
	}
}

void Patcher_ReportFailure()
{
	if( g_Patcher_GettingVersionNumber )
	{
		AddStatus( "Could not connect to update server.", g_StatusColor );
	}
	else if( g_Patcher_GettingManifestFile )
	{
		AddStatus( "Error receiving manifest file.", g_StatusWarningColor );
		++g_NumWarnings;
	}
	else if( g_Patcher_GettingNextPatchFile )
	{
		AddStatus( SimpleString::PrintF( "Error receiving %s.", g_FilesInManifest[ g_NextPatchFileIndex ] ), g_StatusWarningColor );
		++g_NumWarnings;
	}
	else
	{
		AddStatus( "Unknown error connecting to update server.", g_StatusWarningColor );
		++g_NumWarnings;
	}
}

void Patcher_GetVersionNumber()
{
	AddStatus( "Checking for updates...", g_StatusColor );

	STATICHASH( ContentSyncer );
	STATICHASH( PatcherHost );
	STATICHASH( PatcherVersionPath );

	g_Patcher_GettingVersionNumber = true;
	g_Patcher_GettingManifestFile = false;
	g_Patcher_GettingNextPatchFile = false;

	g_Patcher_WaitingToConnect = true;
	g_Patcher_WaitingToReceive = false;

	HTTPSocket::SSocketInit SocketInit;

	SocketInit.m_CloseConnection = true;
	SocketInit.m_HostName = ConfigManager::GetString( sPatcherHost, "", sContentSyncer );
	SocketInit.m_Path = ConfigManager::GetString( sPatcherVersionPath, "", sContentSyncer );
	SocketInit.m_Async = true;
	g_HTTPSocket->Connect( SocketInit );
}

void Patcher_GetManifestFile()
{
	AddStatus( "Getting manifest file...", g_StatusColor );

	STATICHASH( ContentSyncer );
	STATICHASH( PatcherHost );
	STATICHASH( PatcherManifestPath );

	g_Patcher_GettingVersionNumber = false;
	g_Patcher_GettingManifestFile = true;
	g_Patcher_GettingNextPatchFile = false;

	g_Patcher_WaitingToConnect = true;
	g_Patcher_WaitingToReceive = false;

	HTTPSocket::SSocketInit SocketInit;

	SocketInit.m_CloseConnection = true;
	SocketInit.m_HostName = ConfigManager::GetString( sPatcherHost, "", sContentSyncer );
	SocketInit.m_Path = ConfigManager::GetString( sPatcherManifestPath, "", sContentSyncer );
	SocketInit.m_Async = true;
	g_HTTPSocket->Connect( SocketInit );
}

void Patcher_GetNextPatchFile()
{
	ASSERT( g_FilesInManifest.Size() );

	// Iterate to find the next valid patch file
	for( ; g_NextPatchFileIndex < g_FilesInManifest.Size(); ++g_NextPatchFileIndex )
	{
		SManifestFile& PatchFile = g_FilesInManifest[ g_NextPatchFileIndex ];
		SimpleString PatchFilename = PatchFile.m_Filename;
		if( FileUtil::Exists( PatchFilename.CStr() ) )
		{
			FileStream ExtantFile( PatchFilename.CStr(), FileStream::EFM_Read );

			int ExtantSize = ExtantFile.Size();
			uint32 ExtantChecksum = Checksum::Adler32( ExtantFile );
			SimpleString ExtantLengthString = SimpleString::PrintF( "%d", ExtantSize );
			SimpleString ExtantChecksumString = SimpleString::PrintF( "0x%08X", ExtantChecksum );

			if( ExtantLengthString == PatchFile.m_Length && ExtantChecksumString == PatchFile.m_Checksum )
			{
				AddStatus( SimpleString::PrintF( "%s is up to date.", PatchFilename.CStr() ), g_StatusColor );
			}
			else
			{
				// This file isn't up to date. Patch it!
				break;
			}
		}
		else
		{
			// This file isn't on disk. Get it!
			break;
		}
	}

	if( g_NextPatchFileIndex < g_FilesInManifest.Size() )
	{
		SimpleString PatchFile = g_FilesInManifest[ g_NextPatchFileIndex ].m_Filename;

		AddStatus( SimpleString::PrintF( "Getting %s...", PatchFile.CStr() ), g_StatusColor );

		STATICHASH( ContentSyncer );
		STATICHASH( PatcherHost );
		STATICHASH( PatcherContentPath );

		g_Patcher_GettingVersionNumber = false;
		g_Patcher_GettingManifestFile = false;
		g_Patcher_GettingNextPatchFile = true;

		g_Patcher_WaitingToConnect = true;
		g_Patcher_WaitingToReceive = false;

		HTTPSocket::SSocketInit SocketInit;

		SocketInit.m_CloseConnection = true;
		SocketInit.m_HostName = ConfigManager::GetString( sPatcherHost, "", sContentSyncer );
		SocketInit.m_Path = SimpleString::PrintF( "%s%s", ConfigManager::GetString( sPatcherContentPath, "", sContentSyncer ), PatchFile.CStr() );
		SocketInit.m_Async = true;
		g_HTTPSocket->Connect( SocketInit );
	}
	else
	{
		// Nothing left to patch! Handle the wrap up.
		FinishPatching();
	}
}

void CopyTempPathFiles()
{
	AddStatus( "Applying patches...", g_StatusColor );

	for( uint PatchFileIndex = 0; PatchFileIndex < g_FilesInManifest.Size(); ++PatchFileIndex )
	{
		SManifestFile& ManifestFile = g_FilesInManifest[ PatchFileIndex ];

		if( ManifestFile.m_Validated )
		{
			// Create the folder if it doesn't exist yet.
			FileUtil::RecursiveMakePath( ManifestFile.m_Filename.CStr() );

			SimpleString PatchFile = ManifestFile.m_Filename;
			SimpleString TempFile = GetTempFileForPatchFile( PatchFile );

			// Handle the special case of updating our own executable
			bool IsSelf = false;
			STATICHASH( ContentSyncer );
			STATICHASH( Self );
			STATICHASH( TempSelf );
			if( PatchFile == ConfigManager::GetString( sSelf, "", sContentSyncer ) )
			{
				PatchFile = ConfigManager::GetString( sTempSelf, "", sContentSyncer );
				IsSelf = true;
			}

			if( FileUtil::Copy( TempFile.CStr(), PatchFile.CStr(), false ) )
			{
				FileUtil::RemoveFile( TempFile.CStr() );

				if( IsSelf )
				{
					g_ShouldRestart = true;
				}
			}
			else
			{
				AddStatus( SimpleString::PrintF( "Failed to copy %s.", PatchFile.CStr() ).CStr(), g_StatusWarningColor );
				++g_NumWarnings;
			}
		}
	}
}

SimpleString GetTempFileForPatchFile( const SimpleString& PatchFile )
{
	return SimpleString::PrintF( "PATCH-%08X", HashedString( PatchFile ) );
}

void BeginUpdating()
{
	g_Patcher_IsUpdating = true;

	Patcher_GetManifestFile();
}

void InitializeConfigFiles()
{
	// Hard-coded filenames, guh
	PackStream::StaticAddPackageFile( "syncer.cpk" );
	ConfigManager::Load( PackStream( "Config/syncer.pcf" ) );
	PackStream::StaticShutDown();
}

void Reinit()
{
	// Reload launcher config
	InitializeConfigFiles();

	STATICHASH( ContentSyncer );
	STATICHASH( Application );
	g_CommandLine->m_App = ConfigManager::GetString( sApplication, "", sContentSyncer );

	ResolveCommandLine();
}

void FinishPatching()
{
	CopyTempPathFiles();

	// Successfully updated!
	g_Patcher_IsUpdating = false;

	Reinit();

	AddStatus( "Update successful!", g_StatusColor );

	g_ShouldLaunchApp = true;
}

bool CheckForRelaunch()
{
	STATICHASH( ContentSyncer );
	STATICHASH( Self );
	STATICHASH( TempSelf );
	STATICHASH( RelaunchArg );

	SimpleString RelaunchArg = ConfigManager::GetString( sRelaunchArg, "", sContentSyncer );
	SimpleString SelfExe = ConfigManager::GetString( sSelf, "", sContentSyncer );
	SimpleString TempSelfExe = ConfigManager::GetString( sTempSelf, "", sContentSyncer );

	if( HasCommandLineArgument( RelaunchArg ) )
	{
		if( FileUtil::Copy( TempSelfExe.CStr(), SelfExe.CStr(), false ) )
		{
			if( RelaunchTrueSelf() )
			{
				return true;
			}
		}
	}
	else
	{
		// Clean up the relauncher if needed
		if( FileUtil::Exists( TempSelfExe.CStr() ) )
		{
			FileUtil::RemoveFile( TempSelfExe.CStr() );
		}
	}

	return false;
}