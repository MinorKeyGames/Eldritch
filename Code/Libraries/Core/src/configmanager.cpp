#include "core.h"
#include "configmanager.h"
#include "configparser.h"
#include "idatastream.h"
#include "stringmanager.h"
#if BUILD_WINDOWS
#include "consolemanager.h"
#endif
#include "filestream.h"
#include "memorystream.h"
#include "reversehash.h"
#include "allocator.h"

// For strlen
#include <string.h>

ConfigManager* ConfigManager::m_Instance = NULL;
const STRING_TYPE ConfigManager::EmptyContext( "" );

Allocator ConfigManager::m_Allocator( "ConfigManager" );
bool ConfigManager::m_UsingAllocator = false;

#if PARANOID_HASH_CHECK
#define GET_STRING_TYPE GetString
#else
#define GET_STRING_TYPE GetHash
#endif

ConfigManager::ConfigManager()
:	m_Vars( m_UsingAllocator ? &m_Allocator : NULL )
,	m_Bindings()
,	m_LastContextWritten( "" )
{
}

ConfigManager::~ConfigManager()
{
	m_Vars.Clear();
	m_Bindings.Clear();
}

ConfigManager* ConfigManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new ConfigManager;
	}
	return m_Instance;
}

void ConfigManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

void ConfigManager::Load( const IDataStream& Stream )
{
	uint Pos = Stream.GetPos();
	if( 'FCCD' == Stream.ReadUInt32() )
	{
		uint NumContexts = Stream.ReadUInt32();
		for( uint ContextIndex = 0; ContextIndex < NumContexts; ++ContextIndex )
		{
			HashedString ContextKey( Stream.ReadUInt32() );
			uint NumVars = Stream.ReadUInt32();

			VarMap& VarMap = GetInstance()->m_Vars[ ContextKey ];

			for( uint VarIndex = 0; VarIndex < NumVars; ++VarIndex )
			{
				ConfigVar ConfigVar;
				HashedString Key( Stream.ReadUInt32() );
				ConfigVar.m_Type = ( ConfigVar::EVarType )Stream.ReadUInt8();

				switch( ConfigVar.m_Type )
				{
				case ConfigVar::EVT_Bool:
					ConfigVar.m_Bool = ( Stream.ReadUInt8() != 0 );
					break;
				case ConfigVar::EVT_Int:
					ConfigVar.m_Int = Stream.ReadInt32();
					break;
				case ConfigVar::EVT_Float:
					ConfigVar.m_Float = Stream.ReadFloat();
					break;
				case ConfigVar::EVT_String:
					{
						uint Length = Stream.ReadUInt32();

						char* pString = AllocateString( Length + 1 );
						Stream.Read( Length, pString );
						pString[ Length ] = '\0';
						StringManager::AddString( StringManager::ESL_Permanent, pString );

						ConfigVar.m_String = pString;
						ConfigVar.m_Hash = Stream.ReadUInt32();

						if( ReverseHash::IsEnabled() )
						{
							ReverseHash::RegisterHash( ConfigVar.m_Hash, ConfigVar.m_String );
						}
					}
					break;
				default:
					WARNDESC( "Bad config var type" );
					break;
				}

				VarMap.Insert( Key, ConfigVar );
			}
		}

	}
	else
	{
		Stream.SetPos( Pos );
		ConfigParser::Parse( Stream );
	}
}

/*static*/ void ConfigManager::LoadTiny( const IDataStream& Stream )
{
	ConfigParser::ParseTiny( Stream );
}

/*static*/ void ConfigManager::LoadTiny( const SimpleString& TinyString )
{
	ConfigParser::ParseTiny( MemoryStream( const_cast< char* >( TinyString.CStr() ), TinyString.Length() + 1 ) );
}

void ConfigManager::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( 'FCCD' );						// Magic ID: DCCF
	Stream.WriteUInt32( GetInstance()->m_Vars.Size() );	// Number of contexts

	const Map< HashedString, VarMap >& Vars = GetInstance()->m_Vars;
	FOR_EACH_MAP( ContextIterator, Vars, HashedString, VarMap )
	{
		const HashedString& ContextKey = ContextIterator.GetKey();
		const VarMap& VarMap = ContextIterator.GetValue();

		Stream.WriteUInt32( ContextKey.GetHash() );	// Hashed context name
		Stream.WriteUInt32( VarMap.Size() );		// Number of vars in context

		FOR_EACH_MAP( VarIterator, VarMap, HashedString, ConfigVar )
		{
			const HashedString& Key = VarIterator.GetKey();
			const ConfigVar& Value = VarIterator.GetValue();

			Stream.WriteUInt32( Key.GetHash() );	// Hashed var name
			Stream.WriteUInt8( ( uint8 )Value.m_Type );

			switch( Value.m_Type )
			{
			case ConfigVar::EVT_Bool:
				Stream.WriteUInt8( Value.m_Bool );
				break;
			case ConfigVar::EVT_Int:
				Stream.WriteInt32( Value.m_Int );
				break;
			case ConfigVar::EVT_Float:
				Stream.WriteFloat( Value.m_Float );
				break;
			case ConfigVar::EVT_String:
				{
					// Doesn't write the terminating null
					uint Length = ( uint )strlen( Value.m_String );
					Stream.WriteUInt32( Length );
					Stream.Write( Length, Value.m_String );
					Stream.WriteUInt32( Value.m_Hash.GetHash() );
				}
				break;
			default:
				WARNDESC( "Bad config var type" );
				break;
			}
		}
	}
}

void ConfigManager::BeginWriting()
{
	GetInstance()->m_LastContextWritten = "";
}

void ConfigManager::Write( const IDataStream& Stream, const SimpleString& Name, const SimpleString& Context /*= EmptyContext*/ )
{
	VarMap& VarMap = GetInstance()->m_Vars[ HASH_STRING( Context ) ];
	ConfigVar& ConfigVar = VarMap[ HASH_STRING( Name ) ];
	if( ConfigVar.m_Type != ConfigVar::EVT_None )
	{
		if( Context != GetInstance()->m_LastContextWritten )
		{
			Stream.PrintF( "\n[%s]\n", Context.CStr() );
			GetInstance()->m_LastContextWritten = Context;
		}

		Stream.PrintF( "%s = ", Name.CStr() );

		switch( ConfigVar.m_Type )
		{
		case ConfigVar::EVT_Bool:
			if( ConfigVar.m_Bool )
			{
				Stream.PrintF( "true\n" );
			}
			else
			{
				Stream.PrintF( "false\n" );
			}
			break;
		case ConfigVar::EVT_Int:
			Stream.PrintF( "%d\n", ConfigVar.m_Int );
			break;
		case ConfigVar::EVT_Float:
			Stream.PrintF( "%f\n", ConfigVar.m_Float );
			break;
		case ConfigVar::EVT_String:
			{
				const SimpleString ConfigString				= ConfigVar.m_String;
				const SimpleString EscapeSequencedString	= ConfigString.EscapeSequenceEncode();
				Stream.PrintF( "\"%s\"\n", EscapeSequencedString.CStr() );
				break;
			}
		}
	}
}

const ConfigVar* const ConfigManager::GetConfigVarForRead( const STRING_TYPE& Name, const STRING_TYPE& Context /*= EmptyContext*/ ) const
{
	const Map< HashedString, VarMap >::Iterator VarMapIter = m_Vars.Search( HASH_STRING( Context ) );
	if( VarMapIter.IsNull() )
	{
		return NULL;
	}
	const Map< HashedString, ConfigVar >::Iterator ConfigVarIter = ( *VarMapIter ).Search( HASH_STRING( Name ) );
	if( ConfigVarIter.IsNull() )
	{
		return NULL;
	}

	ConfigVar* pConfigVar = &( *ConfigVarIter );

#if PARANOID_HASH_CHECK
	if( pConfigVar->m_Type == ConfigVar::EVT_None || pConfigVar->m_Name == "" )
	{
		pConfigVar->m_Name = Name;
	}
	else
	{
		ASSERTDESC( Name == pConfigVar->m_Name, "Hash collision!" );
	}
#endif

	return pConfigVar;
}

ConfigVar& ConfigManager::GetConfigVarForWrite( const STRING_TYPE& Name, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	const HashedString HashedContext = HASH_STRING( Context );
	const HashedString HashedName = HASH_STRING( Name );

	Map< HashedString, VarMap >::Iterator VarMapIter = m_Vars.Search( HashedContext );
	if( VarMapIter.IsNull() )
	{
		if( m_UsingAllocator )
		{
			// Instantiate VarMaps with our memory manager
			VarMapIter = m_Vars.Insert( HashedContext, VarMap( &m_Allocator ) );
		}
		else
		{
			VarMapIter = m_Vars.Insert( HashedContext, VarMap() );
		}
	}

#if PARANOID_HASH_CHECK
	ConfigVar& ConfigVar = ( *VarMapIter )[ HashedName ];
	if( ConfigVar.m_Type == ConfigVar::EVT_None || ConfigVar.m_Name == "" )
	{
		ConfigVar.m_Name = Name;
	}
	else
	{
		ASSERTDESC( Name == ConfigVar.m_Name, "Hash collision!" );
	}
#else
	return ( *VarMapIter )[ HashedName ];
#endif
}

bool ConfigManager::GetBool( const STRING_TYPE& Name, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	const ConfigVar* const pConfigVar = GetInstance()->GetConfigVarForRead( Name, Context );

#if STRICT_TYPE_CHECK
	DEBUGASSERT( !pConfigVar || pConfigVar->m_Type == ConfigVar::EVT_Bool || pConfigVar->m_Type == ConfigVar::EVT_None );
#endif

	if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Bool )
	{
		return pConfigVar->m_Bool;
	}
	else
	{
		return Default;
	}
}

int ConfigManager::GetInt( const STRING_TYPE& Name, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	const ConfigVar* const pConfigVar = GetInstance()->GetConfigVarForRead( Name, Context );

#if STRICT_TYPE_CHECK
	DEBUGASSERT( !pConfigVar || pConfigVar->m_Type == ConfigVar::EVT_Int || pConfigVar->m_Type == ConfigVar::EVT_None );
#endif

	if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Int )
	{
		return pConfigVar->m_Int;
	}
	else if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Float )
	{
		return static_cast<int>( pConfigVar->m_Float );
	}
	else
	{
		return Default;
	}
}

float ConfigManager::GetFloat( const STRING_TYPE& Name, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	const ConfigVar* const pConfigVar = GetInstance()->GetConfigVarForRead( Name, Context );

#if STRICT_TYPE_CHECK
	DEBUGASSERT( !pConfigVar || pConfigVar->m_Type == ConfigVar::EVT_Float || pConfigVar->m_Type == ConfigVar::EVT_None );
#endif

	if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Float )
	{
		return pConfigVar->m_Float;
	}
	else if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Int )
	{
		return static_cast<float>( pConfigVar->m_Int );
	}
	else
	{
		return Default;
	}
}

const char* ConfigManager::GetString( const STRING_TYPE& Name, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	const ConfigVar* const pConfigVar = GetInstance()->GetConfigVarForRead( Name, Context );

#if STRICT_TYPE_CHECK
	DEBUGASSERT( !pConfigVar || pConfigVar->m_Type == ConfigVar::EVT_String || pConfigVar->m_Type == ConfigVar::EVT_None );
#endif

	if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_String )
	{
		return pConfigVar->m_String;
	}
	else
	{
		return Default;
	}
}

HashedString ConfigManager::GetHash( const STRING_TYPE& Name, const HashedString& Default /*= HashedString::NullString*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	const ConfigVar* const pConfigVar = GetInstance()->GetConfigVarForRead( Name, Context );

#if STRICT_TYPE_CHECK
	DEBUGASSERT( !pConfigVar || pConfigVar->m_Type == ConfigVar::EVT_String || pConfigVar->m_Type == ConfigVar::EVT_None );
#endif

	if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_String )
	{
		return pConfigVar->m_Hash;
	}
	else
	{
		return Default;
	}
}

/*static*/ const char* ConfigManager::GetLanguage()
{
	STATICHASH( Language );
	return GetString( sLanguage, "English" );
}

/*static*/ HashedString ConfigManager::GetLanguageHash()
{
	STATICHASH( Language );
	STATICHASH( English );
	return GetHash( sLanguage, sEnglish );
}

/*static*/ const char* ConfigManager::GetLocalizedString( const STRING_TYPE& Name, const char* Default /*= NULL*/ )
{
#if PARANOID_HASH_CHECK
	// Default to English if the string isn't present in the current language; this way, we avoid showing empty strings.
	return GetString( Name, GetString( Name, Default, "English" ), GetLanguage() );
#else
	// Default to English if the string isn't present in the current language; this way, we avoid showing empty strings.
	STATICHASH( English );
	return GetString( Name, GetString( Name, Default, sEnglish ), GetLanguageHash() );
#endif
}

/*static*/ void ConfigManager::SetLocalizedString( const STRING_TYPE& Name, const char* Value )
{
#if PARANOID_HASH_CHECK
	SetString( Name, Value, GetLanguage() );
#else
	SetString( Name, Value, GetLanguageHash() );
#endif
}

/*static*/ bool ConfigManager::GetArchetypeBool( const STRING_TYPE& Name, const STRING_TYPE& Archetype, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetBool( Name, GetBool( Name, Default, Archetype ), Context );
}

/*static*/ int ConfigManager::GetArchetypeInt( const STRING_TYPE& Name, const STRING_TYPE& Archetype, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInt( Name, GetInt( Name, Default, Archetype ), Context );
}

/*static*/ float ConfigManager::GetArchetypeFloat( const STRING_TYPE& Name, const STRING_TYPE& Archetype, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetFloat( Name, GetFloat( Name, Default, Archetype ), Context );
}

/*static*/ const char* ConfigManager::GetArchetypeString( const STRING_TYPE& Name, const STRING_TYPE& Archetype, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetString( Name, GetString( Name, Default, Archetype ), Context );
}

/*static*/ HashedString ConfigManager::GetArchetypeHash( const STRING_TYPE& Name, const STRING_TYPE& Archetype, const HashedString& Default /*= HashedString::NullString*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetHash( Name, GetHash( Name, Default, Archetype ), Context );
}

/*static*/ bool ConfigManager::GetSequenceBool( const SimpleString& FormatName, int Index, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetBool( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ int ConfigManager::GetSequenceInt( const SimpleString& FormatName, int Index, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInt( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ float ConfigManager::GetSequenceFloat( const SimpleString& FormatName, int Index, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetFloat( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ const char* ConfigManager::GetSequenceString( const SimpleString& FormatName, int Index, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetString( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ HashedString ConfigManager::GetSequenceHash( const SimpleString& FormatName, int Index, const HashedString& Default /*= HashedString::NullString*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetHash( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ bool ConfigManager::GetArchetypeSequenceBool( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetArchetypeBool( SimpleString::PrintF( FormatName.CStr(), Index ), Archetype, Default, Context );
}

/*static*/ int ConfigManager::GetArchetypeSequenceInt( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetArchetypeInt( SimpleString::PrintF( FormatName.CStr(), Index ), Archetype, Default, Context );
}

/*static*/ float ConfigManager::GetArchetypeSequenceFloat( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetArchetypeFloat( SimpleString::PrintF( FormatName.CStr(), Index ), Archetype, Default, Context );
}

/*static*/ const char* ConfigManager::GetArchetypeSequenceString( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetArchetypeString( SimpleString::PrintF( FormatName.CStr(), Index ), Archetype, Default, Context );
}

/*static*/ HashedString ConfigManager::GetArchetypeSequenceHash( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, const HashedString& Default /*= HashedString::NullString*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetArchetypeHash( SimpleString::PrintF( FormatName.CStr(), Index ), Archetype, Default, Context );
}

/*static*/ bool ConfigManager::GetInheritedBool( const STRING_TYPE& Name, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	STRING_TYPE NextContext = Context;
	const ConfigVar* pConfigVar = NULL;
	const ConfigManager* const pInstance = GetInstance();
	for(;;)
	{
		pConfigVar = pInstance->GetConfigVarForRead( Name, NextContext );
		if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Bool )
		{
			// Base case 1: found a matching var
			return pConfigVar->m_Bool;
		}
		else
		{
			STATICHASH( Extends );
			NextContext = GET_STRING_TYPE( sExtends, EmptyContext, NextContext );

			if( NextContext == EmptyContext )
			{
				// Base case 2: found no matches up tree
				return Default;
			}
		}
	}
}

/*static*/ int ConfigManager::GetInheritedInt( const STRING_TYPE& Name, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	STRING_TYPE NextContext = Context;
	const ConfigVar* pConfigVar = NULL;
	const ConfigManager* const pInstance = GetInstance();
	for(;;)
	{
		pConfigVar = pInstance->GetConfigVarForRead( Name, NextContext );
		if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Int )
		{
			// Base case 1: found a matching var
			return pConfigVar->m_Int;
		}
		else if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Float )
		{
			// Base case 1.5: found a matching var
			return static_cast<int>( pConfigVar->m_Float );
		}
		else
		{
			STATICHASH( Extends );
			NextContext = GET_STRING_TYPE( sExtends, EmptyContext, NextContext );

			if( NextContext == EmptyContext )
			{
				// Base case 2: found no matches up tree
				return Default;
			}
		}
	}
}

/*static*/ float ConfigManager::GetInheritedFloat( const STRING_TYPE& Name, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	STRING_TYPE NextContext = Context;
	const ConfigVar* pConfigVar = NULL;
	const ConfigManager* const pInstance = GetInstance();
	for(;;)
	{
		pConfigVar = pInstance->GetConfigVarForRead( Name, NextContext );
		if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Float )
		{
			// Base case 1: found a matching var
			return pConfigVar->m_Float;
		}
		else if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_Int )
		{
			// Base case 1.5: found a matching var
			return static_cast<float>( pConfigVar->m_Int );
		}
		else
		{
			STATICHASH( Extends );
			NextContext = GET_STRING_TYPE( sExtends, EmptyContext, NextContext );

			if( NextContext == EmptyContext )
			{
				// Base case 2: found no matches up tree
				return Default;
			}
		}
	}
}

/*static*/ const char* ConfigManager::GetInheritedString( const STRING_TYPE& Name, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	STRING_TYPE NextContext = Context;
	const ConfigVar* pConfigVar = NULL;
	const ConfigManager* const pInstance = GetInstance();
	for(;;)
	{
		pConfigVar = pInstance->GetConfigVarForRead( Name, NextContext );
		if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_String )
		{
			// Base case 1: found a matching var
			return pConfigVar->m_String;
		}
		else
		{
			STATICHASH( Extends );
			NextContext = GET_STRING_TYPE( sExtends, EmptyContext, NextContext );

			if( NextContext == EmptyContext )
			{
				// Base case 2: found no matches up tree
				return Default;
			}
		}
	}
}

/*static*/ HashedString ConfigManager::GetInheritedHash( const STRING_TYPE& Name, const HashedString& Default /*= HashedString::NullString*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	STRING_TYPE NextContext = Context;
	const ConfigVar* pConfigVar = NULL;
	const ConfigManager* const pInstance = GetInstance();
	for(;;)
	{
		pConfigVar = pInstance->GetConfigVarForRead( Name, NextContext );
		if( pConfigVar && pConfigVar->m_Type == ConfigVar::EVT_String )
		{
			// Base case 1: found a matching var
			return pConfigVar->m_Hash;
		}
		else
		{
			STATICHASH( Extends );
			NextContext = GET_STRING_TYPE( sExtends, EmptyContext, NextContext );

			if( NextContext == EmptyContext )
			{
				// Base case 2: found no matches up tree
				return Default;
			}
		}
	}
}

/*static*/ bool ConfigManager::GetInheritedSequenceBool( const SimpleString& FormatName, int Index, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInheritedBool( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ int ConfigManager::GetInheritedSequenceInt( const SimpleString& FormatName, int Index, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInheritedInt( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ float ConfigManager::GetInheritedSequenceFloat( const SimpleString& FormatName, int Index, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInheritedFloat( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ const char* ConfigManager::GetInheritedSequenceString( const SimpleString& FormatName, int Index, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInheritedString( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

/*static*/ HashedString ConfigManager::GetInheritedSequenceHash( const SimpleString& FormatName, int Index, const HashedString& Default /*= HashedString::NullString*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	return GetInheritedHash( SimpleString::PrintF( FormatName.CStr(), Index ), Default, Context );
}

void ConfigManager::SetBool( const STRING_TYPE& Name, bool Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	ConfigVar& ConfigVar = GetInstance()->GetConfigVarForWrite( Name, Context );

	DEBUGASSERT( ConfigVar.m_Type == ConfigVar::EVT_Bool || ConfigVar.m_Type == ConfigVar::EVT_None );

	ConfigVar.m_Type = ConfigVar::EVT_Bool;
	ConfigVar.m_Bool = Value;

	const List< SBoundVar >& Bindings = GetInstance()->m_Bindings;
	FOR_EACH_LIST( BindingIterator, Bindings, SBoundVar )
	{
		SBoundVar& BoundVar = *BindingIterator;
		if( BoundVar.m_Var == &ConfigVar )
		{
			*( ( bool* )BoundVar.m_Addr ) = Value;
		}
	}
}

void ConfigManager::SetInt( const STRING_TYPE& Name, int Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	ConfigVar& ConfigVar = GetInstance()->GetConfigVarForWrite( Name, Context );

	DEBUGASSERT( ConfigVar.m_Type == ConfigVar::EVT_Int || ConfigVar.m_Type == ConfigVar::EVT_None );

	ConfigVar.m_Type = ConfigVar::EVT_Int;
	ConfigVar.m_Int = Value;

	const List< SBoundVar >& Bindings = GetInstance()->m_Bindings;
	FOR_EACH_LIST( BindingIterator, Bindings, SBoundVar )
	{
		SBoundVar& BoundVar = *BindingIterator;
		if( BoundVar.m_Var == &ConfigVar )
		{
			*( ( int* )BoundVar.m_Addr ) = Value;
		}
	}
}

void ConfigManager::SetFloat( const STRING_TYPE& Name, float Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	ConfigVar& ConfigVar = GetInstance()->GetConfigVarForWrite( Name, Context );

	DEBUGASSERT( ConfigVar.m_Type == ConfigVar::EVT_Float || ConfigVar.m_Type == ConfigVar::EVT_None );

	ConfigVar.m_Type = ConfigVar::EVT_Float;
	ConfigVar.m_Float = Value;

	const List< SBoundVar >& Bindings = GetInstance()->m_Bindings;
	FOR_EACH_LIST( BindingIterator, Bindings, SBoundVar )
	{
		SBoundVar& BoundVar = *BindingIterator;
		if( BoundVar.m_Var == &ConfigVar )
		{
			*( ( float* )BoundVar.m_Addr ) = Value;
		}
	}
}

void ConfigManager::SetString( const STRING_TYPE& Name, const char* Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	DEVASSERT( Value );	// Don't allow setting config var strings to NULL

	// Even though we're given a const char*, don't assume it's permanent;
	// make a permanent copy of the string so we can be sure it'll always be good.
	uint Length = (uint)( strlen( Value ) + 1 );
	char* StringCopy = AllocateString( Length );
	strcpy_s( StringCopy, Length, Value );
	StringManager::AddString( StringManager::ESL_Permanent, StringCopy );

	ConfigVar& ConfigVar = GetInstance()->GetConfigVarForWrite( Name, Context );

	DEBUGASSERT( ConfigVar.m_Type == ConfigVar::EVT_String || ConfigVar.m_Type == ConfigVar::EVT_None );

	// Remove the old string--this is usually safe, but could cause problems if
	// the string is getted, setted, and then the getted value dereferenced.
	if( ConfigVar.m_String )
	{
		StringManager::RemoveString( StringManager::ESL_Permanent, ConfigVar.m_String );
		SafeDelete( ConfigVar.m_String );
	}

	ConfigVar.m_Type = ConfigVar::EVT_String;
	ConfigVar.m_String = StringCopy;
	ConfigVar.m_Hash = HashedString( StringCopy );	// Update the hash whenever the string is changed

	const List< SBoundVar >& Bindings = GetInstance()->m_Bindings;
	FOR_EACH_LIST( BindingIterator, Bindings, SBoundVar )
	{
		SBoundVar& BoundVar = *BindingIterator;
		if( BoundVar.m_Var == &ConfigVar )
		{
			*( ( const char** )BoundVar.m_Addr ) = StringCopy;
		}
	}
}

void ConfigManager::ToggleBool( const STRING_TYPE& Name, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	SetBool( Name, !GetBool( Name, false, Context ), Context );
}

void ConfigManager::AddInt( const STRING_TYPE& Name, int Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	SetInt( Name, Value + GetInt( Name, 0, Context ), Context );
}

void ConfigManager::AddFloat( const STRING_TYPE& Name, float Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	SetFloat( Name, Value + GetFloat( Name, 0.0f, Context ), Context );
}

void ConfigManager::AppendString( const STRING_TYPE& Name, const char* Value, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	SetString( Name, SimpleString::PrintF( "%s%s", GetString( Name, "", Context ), Value ).CStr(), Context );
}

void ConfigManager::Bind( bool* Addr, const STRING_TYPE& Name, bool Default /*= false*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	VarMap& VarMap = GetInstance()->m_Vars[ HASH_STRING( Context ) ];
	ConfigVar& ConfigVar = VarMap[ HASH_STRING( Name ) ];
	GetInstance()->m_Bindings.PushBack( SBoundVar( Addr, &ConfigVar ) );

	// Set config var to itself to apply the value to the bound
	// C++ variable (and create the entry if it didn't exist yet).
	SetBool( Name, GetBool( Name, Default ) );
}

void ConfigManager::Bind( int* Addr, const STRING_TYPE& Name, int Default /*= 0*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	VarMap& VarMap = GetInstance()->m_Vars[ HASH_STRING( Context ) ];
	ConfigVar& ConfigVar = VarMap[ HASH_STRING( Name ) ];
	GetInstance()->m_Bindings.PushBack( SBoundVar( Addr, &ConfigVar ) );

	// Set config var to itself to apply the value to the bound
	// C++ variable (and create the entry if it didn't exist yet).
	SetInt( Name, GetInt( Name, Default ) );
}

void ConfigManager::Bind( float* Addr, const STRING_TYPE& Name, float Default /*= 0.0f*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	VarMap& VarMap = GetInstance()->m_Vars[ HASH_STRING( Context ) ];
	ConfigVar& ConfigVar = VarMap[ HASH_STRING( Name ) ];
	GetInstance()->m_Bindings.PushBack( SBoundVar( Addr, &ConfigVar ) );

	// Set config var to itself to apply the value to the bound
	// C++ variable (and create the entry if it didn't exist yet).
	SetFloat( Name, GetFloat( Name, Default ) );
}

void ConfigManager::Bind( const char** Addr, const STRING_TYPE& Name, const char* Default /*= NULL*/, const STRING_TYPE& Context /*= EmptyContext*/ )
{
	VarMap& VarMap = GetInstance()->m_Vars[ HASH_STRING( Context ) ];
	ConfigVar& ConfigVar = VarMap[ HASH_STRING( Name ) ];
	GetInstance()->m_Bindings.PushBack( SBoundVar( Addr, &ConfigVar ) );

	// Set config var to itself to apply the value to the bound
	// C++ variable (and create the entry if it didn't exist yet).
	SetString( Name, GetString( Name, Default ) );
}

void ConfigManager::Unbind( void* Addr )
{
	List< SBoundVar >& Bindings = GetInstance()->m_Bindings;
	FOR_EACH_LIST( BindingIterator, Bindings, SBoundVar )
	{
		if( ( *BindingIterator ).m_Addr == Addr )
		{
			Bindings.Pop( BindingIterator );
			break;
		}
	}
}

void ConfigManager::Report()
{
#if BUILD_DEBUG
	const Map< HashedString, VarMap >& Vars = GetInstance()->m_Vars;
	FOR_EACH_MAP( ContextIterator, Vars, HashedString, VarMap )
	{
		const HashedString& ContextKey = ContextIterator.GetKey();
		const VarMap& VarMap = ContextIterator.GetValue();
		FOR_EACH_MAP( VarIterator, VarMap, HashedString, ConfigVar )
		{
			const HashedString& Key = VarIterator.GetKey();
			const ConfigVar& Value = VarIterator.GetValue();

			DEBUGPRINTF( "0x%08X:0x%08X: ", ContextKey.GetHash(), Key.GetHash() );

			switch( Value.m_Type )
			{
			case ConfigVar::EVT_None:
				DEBUGPRINTF( "Undefined\n" );
				break;
			case ConfigVar::EVT_Bool:
				if( Value.m_Bool )
				{
					DEBUGPRINTF( "true\n" );
				}
				else
				{
					DEBUGPRINTF( "false\n" );
				}
				break;
			case ConfigVar::EVT_Int:
				DEBUGPRINTF( "%d\n", Value.m_Int );
				break;
			case ConfigVar::EVT_Float:
				DEBUGPRINTF( "%f\n", Value.m_Float );
				break;
			case ConfigVar::EVT_String:
				DEBUGPRINTF( "%s\n", Value.m_String );
				break;
			default:
				break;
			}
		}
	}
#endif // BUILD_DEBUG
}

void ConfigManager::Report( const IDataStream& Stream )
{
	Unused( Stream );

#if BUILD_DEV
	Stream.PrintF( "****************************************\n" );
	Stream.PrintF( "Config Manager Report\n" );
	Stream.PrintF( "****************************************\n\n" );

	uint NumContexts = 0;
	uint NumVars = 0;

	const Map< HashedString, VarMap >& Vars = GetInstance()->m_Vars;
	FOR_EACH_MAP( ContextIterator, Vars, HashedString, VarMap )
	{
		++NumContexts;
		const HashedString& ContextKey = ContextIterator.GetKey();
		const VarMap& VarMap = ContextIterator.GetValue();
		FOR_EACH_MAP( VarIterator, VarMap, HashedString, ConfigVar )
		{
			++NumVars;
			const HashedString& Key = VarIterator.GetKey();
			const ConfigVar& Value = VarIterator.GetValue();

			Stream.PrintF( "0x%08X:0x%08X: ", ContextKey.GetHash(), Key.GetHash() );

			switch( Value.m_Type )
			{
			case ConfigVar::EVT_None:
				Stream.PrintF( "Undefined\n" );
				break;
			case ConfigVar::EVT_Bool:
				if( Value.m_Bool )
				{
					Stream.PrintF( "true\n" );
				}
				else
				{
					Stream.PrintF( "false\n" );
				}
				break;
			case ConfigVar::EVT_Int:
				Stream.PrintF( "%d\n", Value.m_Int );
				break;
			case ConfigVar::EVT_Float:
				Stream.PrintF( "%f\n", Value.m_Float );
				break;
			case ConfigVar::EVT_String:
				Stream.PrintF( "%s\n", Value.m_String );
				break;
			default:
				break;
			}
		}
	}

	Stream.PrintF( "\n%d config vars in %d contexts", NumVars, NumContexts );
#endif // BUILD_DEV
}

#if BUILD_WINDOWS
/*static*/ bool ConfigManager::ProcessCommand( const SimpleString& Command )
{
	// Commands to modify config vars from console, mainly intended to support
	// scripting (hence the arithmetic operators).

	// Bool commands
	if( Command == "setbool" )	// setbool name value context
	{
		SimpleString Name = ConsoleManager::GetString();
		bool Value = ConsoleManager::GetBool();
		SimpleString Context = ConsoleManager::GetString();
		SetBool( Name, Value, Context );
		return true;
	}
	else if( Command == "negatebool" )	// negatebool name context
	{
		SimpleString Name = ConsoleManager::GetString();
		SimpleString Context = ConsoleManager::GetString();
		SetBool( Name, !GetBool( Name, false, Context ), Context );
		return true;
	}

	// Int commands
	else if( Command == "setint" )	// setint name value context
	{
		SimpleString Name = ConsoleManager::GetString();
		int Value = ConsoleManager::GetInt();
		SimpleString Context = ConsoleManager::GetString();
		SetInt( Name, Value, Context );
		return true;
	}
	else if( Command == "negateint" )	// negateint name context
	{
		SimpleString Name = ConsoleManager::GetString();
		SimpleString Context = ConsoleManager::GetString();
		SetInt( Name, -GetInt( Name, 0, Context ), Context );
		return true;
	}
	else if( Command == "addint" )	// addint name value context
	{
		SimpleString Name = ConsoleManager::GetString();
		int Value = ConsoleManager::GetInt();
		SimpleString Context = ConsoleManager::GetString();
		SetInt( Name, GetInt( Name, 0, Context ) + Value, Context );
		return true;
	}
	else if( Command == "multiplyint" )	// multiplyint name value context
	{
		SimpleString Name = ConsoleManager::GetString();
		int Value = ConsoleManager::GetInt();
		SimpleString Context = ConsoleManager::GetString();
		SetInt( Name, GetInt( Name, 0, Context ) * Value, Context );
		return true;
	}

	// Float commands
	else if( Command == "setfloat" )
	{
		SimpleString Name = ConsoleManager::GetString();
		float Value = ConsoleManager::GetFloat();
		SimpleString Context = ConsoleManager::GetString();
		SetFloat( Name, Value, Context );
		return true;
	}
	else if( Command == "negatefloat" )	// negatefloat name context
	{
		SimpleString Name = ConsoleManager::GetString();
		SimpleString Context = ConsoleManager::GetString();
		SetFloat( Name, -GetFloat( Name, 0.0f, Context ), Context );
		return true;
	}
	else if( Command == "addfloat" )	// addfloat name value context
	{
		SimpleString Name = ConsoleManager::GetString();
		int Value = ConsoleManager::GetInt();
		SimpleString Context = ConsoleManager::GetString();
		SetFloat( Name, GetFloat( Name, 0.0f, Context ) + Value, Context );
		return true;
	}
	else if( Command == "multiplyfloat" )	// multiplyfloat name value context
	{
		SimpleString Name = ConsoleManager::GetString();
		int Value = ConsoleManager::GetInt();
		SimpleString Context = ConsoleManager::GetString();
		SetFloat( Name, GetFloat( Name, 0.0f, Context ) * Value, Context );
		return true;
	}

	// String commands
	else if( Command == "setstring" )
	{
		SimpleString Name = ConsoleManager::GetString();
		const char* Value = ConsoleManager::GetString().CStr();
		SimpleString Context = ConsoleManager::GetString();
		SetString( Name, Value, Context );
		return true;
	}

#if BUILD_DEV
	else if( Command == "config_report" )
	{
		SimpleString ReportFilename = ConsoleManager::GetString();
		if( ReportFilename == "" )
		{
			ReportFilename = "config-report.txt";
		}
		Report( FileStream( ReportFilename.CStr(), FileStream::EFM_Write ) );
		return true;
	}
#endif
	else
	{
		return false;
	}
}
#endif

/*static*/ void ConfigManager::InitializeAllocator( uint Size )
{
	m_Allocator.Initialize( Size );
	m_UsingAllocator = true;
}

/*static*/ void ConfigManager::ShutDownAllocator()
{
#if BUILD_DEBUG
	m_Allocator.Report( FileStream( "memory_exit_report.txt", FileStream::EFM_Append ) );
#endif
	DEBUGASSERT( m_Allocator.CheckForLeaks() );
	m_Allocator.ShutDown();
	m_UsingAllocator = false;
}

/*static*/ char* ConfigManager::AllocateString( uint Size )
{
	if( m_UsingAllocator )
	{
		return new( m_Allocator ) char[ Size ];
	}
	else
	{
		return new char[ Size ];
	}
}

/*static*/ void ConfigManager::ReportAllocator( const SimpleString& Filename )
{
	Unused( Filename );
#if BUILD_DEBUG
	m_Allocator.Report( FileStream( Filename.CStr(), FileStream::EFM_Append ) );
#endif
}