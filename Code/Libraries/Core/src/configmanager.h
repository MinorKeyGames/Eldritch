#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "configvar.h"
#include "simplestring.h"
#include "hashedstring.h"
#include "map.h"
#include "list.h"

// Maintains a map of ConfigVars and is the user interface to the
// configuration variable system.

#if PARANOID_HASH_CHECK
#define STRING_TYPE SimpleString
#define HASH_STRING(s) (HashedString(s))
#else
#define STRING_TYPE HashedString
#define HASH_STRING(s) (s)
#endif

#define STATICHASH(name) static const STRING_TYPE s##name(#name)
#define MAKEHASH(name) const STRING_TYPE s##name(name)
#define MAKEHASHFROM(name, src) const STRING_TYPE s##name(src)

class IDataStream;

class ConfigManager
{
public:
	static ConfigManager*	GetInstance();
	static void				DeleteInstance();

	// Load and save binary representations
	static void				Load( const IDataStream& Stream );
	static void				Save( const IDataStream& Stream );

	static void				LoadTiny( const IDataStream& Stream );
	static void				LoadTiny( const SimpleString& TinyString );

	// Write text representations
	static void				BeginWriting();	// Just resets the last context member
	static void				Write( const IDataStream& Stream, const SimpleString& Name, const SimpleString& Context = "" );

	static bool				GetBool( const STRING_TYPE& Name, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static int				GetInt( const STRING_TYPE& Name, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static float			GetFloat( const STRING_TYPE& Name, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static const char*		GetString( const STRING_TYPE& Name, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static HashedString		GetHash( const STRING_TYPE& Name, const HashedString& Default = HashedString::NullString, const STRING_TYPE& Context = EmptyContext );

	// Helper for languages--no need for a context here
	static const char*		GetLanguage();
	static HashedString		GetLanguageHash();
	static const char*		GetLocalizedString( const STRING_TYPE& Name, const char* Default = NULL );
	static void				SetLocalizedString( const STRING_TYPE& Name, const char* Value );

	// Helpers for archetypes (referencing another context for a default value)
	static bool				GetArchetypeBool( const STRING_TYPE& Name, const STRING_TYPE& Archetype, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static int				GetArchetypeInt( const STRING_TYPE& Name, const STRING_TYPE& Archetype, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static float			GetArchetypeFloat( const STRING_TYPE& Name, const STRING_TYPE& Archetype, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static const char*		GetArchetypeString( const STRING_TYPE& Name, const STRING_TYPE& Archetype, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static HashedString		GetArchetypeHash( const STRING_TYPE& Name, const STRING_TYPE& Archetype, const HashedString& Default = HashedString::NullString, const STRING_TYPE& Context = EmptyContext );

	// Helpers for sequences (where the name is expected to be formatted with a %d for the index)
	static bool				GetSequenceBool( const SimpleString& FormatName, int Index, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static int				GetSequenceInt( const SimpleString& FormatName, int Index, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static float			GetSequenceFloat( const SimpleString& FormatName, int Index, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static const char*		GetSequenceString( const SimpleString& FormatName, int Index, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static HashedString		GetSequenceHash( const SimpleString& FormatName, int Index, const HashedString& Default = HashedString::NullString, const STRING_TYPE& Context = EmptyContext );

	// Helpers for archetype sequences
	static bool				GetArchetypeSequenceBool( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static int				GetArchetypeSequenceInt( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static float			GetArchetypeSequenceFloat( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static const char*		GetArchetypeSequenceString( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static HashedString		GetArchetypeSequenceHash( const SimpleString& FormatName, int Index, const STRING_TYPE& Archetype, const HashedString& Default = HashedString::NullString, const STRING_TYPE& Context = EmptyContext );

	// Helpers for inherited values (where a context references a parent context via Extends var)
	static bool				GetInheritedBool( const STRING_TYPE& Name, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static int				GetInheritedInt( const STRING_TYPE& Name, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static float			GetInheritedFloat( const STRING_TYPE& Name, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static const char*		GetInheritedString( const STRING_TYPE& Name, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static HashedString		GetInheritedHash( const STRING_TYPE& Name, const HashedString& Default = HashedString::NullString, const STRING_TYPE& Context = EmptyContext );

	// Helpers for inherited sequences
	static bool				GetInheritedSequenceBool( const SimpleString& FormatName, int Index, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static int				GetInheritedSequenceInt( const SimpleString& FormatName, int Index, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static float			GetInheritedSequenceFloat( const SimpleString& FormatName, int Index, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static const char*		GetInheritedSequenceString( const SimpleString& FormatName, int Index, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static HashedString		GetInheritedSequenceHash( const SimpleString& FormatName, int Index, const HashedString& Default = HashedString::NullString, const STRING_TYPE& Context = EmptyContext );

	// Setter functions (to create or redefine config vars)
	// Explicitly typed to prevent accidentally redefining a type
	static void				SetBool( const STRING_TYPE& Name, bool Value, const STRING_TYPE& Context = EmptyContext );
	static void				SetInt( const STRING_TYPE& Name, int Value, const STRING_TYPE& Context = EmptyContext );
	static void				SetFloat( const STRING_TYPE& Name, float Value, const STRING_TYPE& Context = EmptyContext );
	static void				SetString( const STRING_TYPE& Name, const char* Value, const STRING_TYPE& Context = EmptyContext );

	// Helper functions for modifying config values in-place
	static void				ToggleBool( const STRING_TYPE& Name, const STRING_TYPE& Context = EmptyContext );
	static void				AddInt( const STRING_TYPE& Name, int Value, const STRING_TYPE& Context = EmptyContext );
	static void				AddFloat( const STRING_TYPE& Name, float Value, const STRING_TYPE& Context = EmptyContext );
	static void				AppendString( const STRING_TYPE& Name, const char* Value, const STRING_TYPE& Context = EmptyContext );

	static void				Bind( bool* Addr, const STRING_TYPE& Name, bool Default = false, const STRING_TYPE& Context = EmptyContext );
	static void				Bind( int* Addr, const STRING_TYPE& Name, int Default = 0, const STRING_TYPE& Context = EmptyContext );
	static void				Bind( float* Addr, const STRING_TYPE& Name, float Default = 0.0f, const STRING_TYPE& Context = EmptyContext );
	static void				Bind( const char** Addr, const STRING_TYPE& Name, const char* Default = NULL, const STRING_TYPE& Context = EmptyContext );
	static void				Unbind( void* Addr );

	static void				Report();
	static void				Report( const IDataStream& Stream );

#if BUILD_WINDOWS
	static bool				ProcessCommand( const SimpleString& Command );
#endif

	static void				InitializeAllocator( uint Size );
	static void				ShutDownAllocator();
	static void				ReportAllocator( const SimpleString& Filename );

	static char*			AllocateString( uint Size );

	static const STRING_TYPE	EmptyContext;

private:
	ConfigManager();
	~ConfigManager();

	// Access a config var for reading, returning NULL if it doesn't exist
	const ConfigVar* const	GetConfigVarForRead( const STRING_TYPE& Name, const STRING_TYPE& Context = EmptyContext ) const;

	// Access a config var for writing, creating it if it doesn't exist
	ConfigVar&				GetConfigVarForWrite( const STRING_TYPE& Name, const STRING_TYPE& Context = EmptyContext );

	static ConfigManager*			m_Instance;

	typedef Map< HashedString, ConfigVar > VarMap;
	Map< HashedString, VarMap >		m_Vars;	// 2-D map--contexts by config names

	struct SBoundVar
	{
		SBoundVar( const void* Addr = NULL, ConfigVar* Var = NULL ) : m_Addr( Addr ), m_Var( Var ) {}
		const void*	m_Addr;
		ConfigVar*	m_Var;
	};
	List< SBoundVar >				m_Bindings;

	SimpleString					m_LastContextWritten;

	static Allocator			m_Allocator;
	static bool						m_UsingAllocator;
};

#endif // CONFIGMANAGER_H