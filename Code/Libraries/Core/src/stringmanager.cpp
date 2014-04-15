#include "core.h"
#include "stringmanager.h"
#include "array.h"
#include "configmanager.h"
#include "configparser.h"
#include "memorystream.h"
#include "filestream.h"
#include "allocator.h"

#include <stdio.h>
#include <stdarg.h>

StringManager*	StringManager::m_Instance = NULL;
Allocator		StringManager::m_AllocatorPermanent( "StringManager/Permanent" );
Allocator		StringManager::m_AllocatorTransient( "StringManager/Transient" );
bool			StringManager::m_UsingAllocator = false;

StringManager::StringManager()
:	m_Strings()
{
}

StringManager::~StringManager()
{
	FlushStrings( ESL_Permanent );
}

const char* StringManager::FormatV( EStringLife Life, const char* FormatString, va_list Args )
{
	int Length;
	char* Buffer;

	Length = VSPRINTF_COUNT( FormatString, Args ) + 1;
	Buffer = Allocate( Life, Length );
	VSPRINTF( Buffer, Length, FormatString, Args );
	AddString( Life, Buffer );
	return Buffer;
}

const char* StringManager::PrintF( EStringLife Life, const char* FormatString, ... )
{
	va_list Args;
	va_start( Args, FormatString );
	return FormatV( Life, FormatString, Args );
}

// Parses strings containing tokens of the form "#{[type]<:context>:[name]}", where type is:
// b: bool (replaced with localized True/False)
// a: bool (replaced with localized Achieved/Not Achieved
// i: signed int
// x: int as unsigned hex
// t: int as 2 digits for time
// f: float
// s: string
// l: localized string
// Localized string looks up any config var string, then looks up that
// *name* in the string table. Else it wouldn't be much use.
const char* StringManager::ParseConfigString( EStringLife Life, const char* ConfigString )
{
	ASSERT( ConfigString );

	Array< char > ParsedString;

	while( *ConfigString )
	{
		char NextChar = *ConfigString++;
		if( NextChar == '#' )
		{
			// Maybe a token!!
			NextChar = *ConfigString++;
			if( NextChar == '{' )
			{
				// A token!!
				ParseConfigToken( ParsedString, ConfigString );
			}
			else
			{
				ParsedString.PushBack( '#' );
				ParsedString.PushBack( NextChar );
			}
		}
		else
		{
			ParsedString.PushBack( NextChar );
		}
	}
	ParsedString.PushBack( '\0' );

	// Make a permanent copy of the string
	char* TheString = Allocate( Life, ParsedString.Size() );
	memcpy( TheString, ParsedString.GetData(), ParsedString.Size() );
	AddString( Life, TheString );
	return TheString;
}

void StringManager::ParseConfigToken( Array< char >& ParsedString, const char*& ConfigString )
{
	Array< char > FirstPartArray;	// Could be name or context
	Array< char > SecondPartArray;	// Name is first part is context
	bool HasContext = false;

	char TypeChar = *ConfigString++;
	ASSERT( *ConfigString == ':' );
	ConfigString++;

	while( *ConfigString )
	{
		char NextNameChar = *ConfigString++;
		if( NextNameChar == ':' )
		{
			HasContext = true;
			break;
		}
		else if( NextNameChar == '}' )
		{
			break;
		}
		else
		{
			FirstPartArray.PushBack( NextNameChar );
		}
	}
	FirstPartArray.PushBack( '\0' );

	if( HasContext )
	{
		while( *ConfigString )
		{
			char NextContextChar = *ConfigString++;
			if( NextContextChar == '}' )
			{
				break;
			}
			else
			{
				SecondPartArray.PushBack( NextContextChar );
			}
		}
	}
	SecondPartArray.PushBack( '\0' );

	SimpleString Name = ( SecondPartArray.Size() > 1 ) ? SecondPartArray.GetData() : FirstPartArray.GetData();
	SimpleString Context = ( SecondPartArray.Size() > 1 ) ? FirstPartArray.GetData() : "";

	const char* AppendString = NULL;

	if( TypeChar == 'b' )
	{
		bool TheBool = ConfigManager::GetBool( Name, false, Context );
		AppendString = ConfigManager::GetLocalizedString( ( TheBool ? "True" : "False" ), "" );
	}
	else if( TypeChar == 'a' )
	{
		bool TheBool = ConfigManager::GetBool( Name, false, Context );
		AppendString = ConfigManager::GetLocalizedString( ( TheBool ? "Achieved" : "NotAchieved" ), "" );
	}
	else if( TypeChar == 'i' )
	{
		int TheInt = ConfigManager::GetInt( Name, 0, Context );
		AppendString = PrintF( ESL_Transient, "%d", TheInt );
	}
	else if( TypeChar == 't' )
	{
		int TheInt = ConfigManager::GetInt( Name, 0, Context );
		AppendString = PrintF( ESL_Transient, "%02d", TheInt );
	}
	else if( TypeChar == 'x' )
	{
		int TheInt = ConfigManager::GetInt( Name, 0, Context );
		AppendString = PrintF( ESL_Transient, "0x%08X", TheInt );
	}
	else if( TypeChar == 'f' )
	{
		float TheFloat = ConfigManager::GetFloat( Name, 0.0f, Context );
		AppendString = PrintF( ESL_Transient, "%.2f", TheFloat );	// TODO: Might want to format this differently
	}
	else if( TypeChar == 's' )
	{
		AppendString = ConfigManager::GetString( Name, "", Context );
	}
	else if( TypeChar == 'l' )
	{
		AppendString = ConfigManager::GetLocalizedString( ConfigManager::GetString( Name, "", Context ), "" );
	}
	else if( TypeChar == 'c' )
	{
		AppendString = ConfigManager::GetLocalizedString( ConfigManager::GetString( Name, "", Context ), "" );
		AppendString = ParseConfigString( ESL_Transient, AppendString );
	}

	ASSERT( AppendString );

	while( *AppendString )
	{
		ParsedString.PushBack( *AppendString++ );
	}
}

/*static*/ bool StringManager::ResolveAndEvaluateConditional( const char* ConditionalString )
{
	// Parse instruction for config vars, if any, and then evaluate as a conditional
	SimpleString Expression = ParseConfigString( ESL_Transient, ConditionalString );
	return ConfigParser::EvaluateConditional( MemoryStream( const_cast< char* >( Expression.CStr() ), Expression.Length() + 1 ) );
}

/*static*/ void StringManager::AddString( EStringLife Life, const char* String )
{
	GetInstance()->m_Strings[ Life ].Insert( String );
}

/*static*/ void StringManager::FlushStrings( EStringLife Life )
{
	GetInstance()->_FlushStrings( Life );
}

/*static*/ void StringManager::RemoveString( EStringLife Life, const char* StringToRemove )
{
	GetInstance()->m_Strings[ Life ].Remove( StringToRemove );
}

void StringManager::_FlushStrings( EStringLife Life )
{
	FOR_EACH_MAP( SetIter, m_Strings, EStringLife, Set< const char* > )
	{
		if( SetIter.GetKey() <= Life )
		{
			Set< const char* >& LifeSet = *SetIter;
			FOR_EACH_SET( StringIter, LifeSet, const char* )
			{
				SafeDelete( *StringIter );
			}
			LifeSet.Clear();
		}
	}
}

StringManager* StringManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new StringManager;
	}
	return m_Instance;
}

void StringManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

char* StringManager::Allocate( EStringLife Life, uint Size )
{
	if( m_UsingAllocator )
	{
		return new( GetAllocator( Life ) ) char[ Size ];
	}
	else
	{
		return new char[ Size ];
	}
}

Allocator& StringManager::GetAllocator( EStringLife Life )
{
	if( Life == ESL_Permanent )
	{
		return m_AllocatorPermanent;
	}
	else
	{
		return m_AllocatorTransient;
	}
}

/*static*/ void StringManager::InitializeAllocator( uint Size )
{
	m_AllocatorPermanent.Initialize( Size >> 1 );
	m_AllocatorTransient.Initialize( Size >> 1 );

	m_UsingAllocator = true;
}

/*static*/ void StringManager::ShutDownAllocator()
{
#if BUILD_DEBUG
	m_AllocatorPermanent.Report( FileStream( "memory_exit_report.txt", FileStream::EFM_Append ) );
	m_AllocatorTransient.Report( FileStream( "memory_exit_report.txt", FileStream::EFM_Append ) );
#endif

	DEBUGASSERT( m_AllocatorPermanent.CheckForLeaks() );
	DEBUGASSERT( m_AllocatorTransient.CheckForLeaks() );

	m_AllocatorPermanent.ShutDown();
	m_AllocatorTransient.ShutDown();

	m_UsingAllocator = false;
}

/*static*/ void StringManager::ReportAllocator( const SimpleString& Filename )
{
	Unused( Filename );
#if BUILD_DEBUG
	m_AllocatorPermanent.Report( FileStream( Filename.CStr(), FileStream::EFM_Append ) );
	m_AllocatorTransient.Report( FileStream( Filename.CStr(), FileStream::EFM_Append ) );
#endif
}