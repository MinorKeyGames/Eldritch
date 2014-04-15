#ifndef STRINGMANAGER_H
#define STRINGMANAGER_H

#include "map.h"
#include "set.h"
#include "array.h"

#include <stdarg.h>

class StringManager
{
public:
	enum EStringLife
	{
		ESL_Transient,	// Flushed every frame
		ESL_Permanent,	// Flushed when StringManager is shut down
	};

	static const char* FormatV( EStringLife Life, const char* FormatString, va_list Args );
	static const char* PrintF( EStringLife Life, const char* FormatString, ... );
	static const char* ParseConfigString( EStringLife Life, const char* ConfigString );	// In this case, a "config string" has config var tokens embedded
	static bool ResolveAndEvaluateConditional( const char* ConditionalString );			// This isn't really the best place for this, but it kinda doesn't fit anywhere

	static void AddString( EStringLife Life, const char* String );
	static void FlushStrings( EStringLife Life );
	static void RemoveString( EStringLife Life, const char* StringToRemove );	// Life needed as hint for where string is stored; if this is a problem, I could make a different function to search all lifetimes

	static StringManager* GetInstance();
	static void DeleteInstance();

	static void			InitializeAllocator( uint Size );
	static void			ShutDownAllocator();
	static void			ReportAllocator( const SimpleString& Filename );

private:
	StringManager();
	~StringManager();

	void _FlushStrings( EStringLife Life );
	Map< EStringLife, Set< const char* > >	m_Strings;

	static StringManager*	m_Instance;

	static void ParseConfigToken( Array< char >& ParsedString, const char*& ConfigString );

	static char*			Allocate( EStringLife Life, uint Size );
	static Allocator&	GetAllocator( EStringLife Life );

	static Allocator	m_AllocatorPermanent;
	static Allocator	m_AllocatorTransient;
	static bool				m_UsingAllocator;
};

#endif