#ifndef PRINTMANAGER_H
#define PRINTMANAGER_H

#include "hashedstring.h"

class IDataStream;
class FileStream;

// If defined, we open log file for each print.
// We always do this in debug mode.
// This lets two instances of the game run concurrently.
#define ALWAYSOPENLOGFILE 1
#define OPENLOGFILE ( BUILD_DEBUG || ALWAYSOPENLOGFILE )

// Print levels:
// -1 = get no output
// 0 = anything that should always be printed, such as major system alerts or specifically requested output
// 1 = non-spammy informational output
// 2 = spammy information output
#define PRINTLEVEL_None		-1
#define PRINTLEVEL_Normal	0
#define PRINTLEVEL_Info		1
#define PRINTLEVEL_Spam		2

#define PRINTCHANNEL_Console	1
#define PRINTCHANNEL_Output		2
#define PRINTCHANNEL_Log		4

// Categories:
// Defined empirically by their use in code and whatever levels are
// listened to in config files.
#define SETCATPRINTLEVEL( cat, level ) PrintManager::GetInstance()->SetPrintLevel( cat, level )
#define SETPRINTLEVEL( level ) PrintManager::GetInstance()->SetPrintLevel( HashedString::NullString, level )

// Non-debug printing is mainly intended for tools, where I want output in Release mode
#define CATPRINTF( cat, level, ... ) PrintManager::GetInstance()->Printf( cat, level, __VA_ARGS__ )
#define LEVELPRINTF( level, ... ) PrintManager::GetInstance()->Printf( HashedString::NullString, level, __VA_ARGS__ )
#define PRINTF( ... ) PrintManager::GetInstance()->Printf( HashedString::NullString, 0, __VA_ARGS__ )

#if BUILD_DEBUG
#define DEBUGCATPRINTF( cat, level, ... ) PrintManager::GetInstance()->Printf( cat, level, __VA_ARGS__ )
#define DEBUGLEVELPRINTF( level, ... ) PrintManager::GetInstance()->Printf( HashedString::NullString, level, __VA_ARGS__ )
#define DEBUGPRINTF( ... ) PrintManager::GetInstance()->Printf( HashedString::NullString, 0, __VA_ARGS__ )
#else
#define DEBUGCATPRINTF( cat, level, format, ... ) 0
#define DEBUGLEVELPRINTF( level, format, ... ) 0
#define DEBUGPRINTF( format, ... ) 0
#endif // BUILD_DEBUG

#define SETPRINTCHANNELS( channels ) PrintManager::GetInstance()->SetChannels( channels )

#define PRINTLOGS( title ) PrintManager::GetInstance()->LogTo( #title "-log.txt" )	// Hacky, but enforces using static strings for title

#define LOADPRINTLEVELS PrintManager::GetInstance()->LoadPrintLevels()

class PrintManager
{
public:
	static PrintManager*	GetInstance();
	static PrintManager*	GetInstance_NoAlloc();
	static void				DeleteInstance();

	void					SetPrintLevel( const HashedString& Category, int Level );
	void					Printf( const HashedString& Category, int Level, const char* Format, ... );

	void					SetChannels( int Channels );

	void					LogTo( const char* Filename );

	void					LoadPrintLevels();

#if OPENLOGFILE
	const char*				GetLogFilename() const { return m_LogFilename; }
#else
	const char*				GetLogFilename() const { WARN; return NULL; }
#endif

private:
	PrintManager();
	~PrintManager();

	static PrintManager*		m_Instance;

	char*						m_StringBuffer;

	int							m_Channels;

#if OPENLOGFILE
	const char*					m_LogFilename;
#else
	IDataStream*				m_LogStream;	// In Release/Final, open stream once and keep it open. In Debug, open for append for every call.
#endif
};

#endif // PRINTMANAGER_H