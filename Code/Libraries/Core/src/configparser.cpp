#include "core.h"
#include "configparser.h"
#include "idatastream.h"
#include "array.h"
#include "configmanager.h"
#include "stringmanager.h"
#include "list.h"

// Syntax:
// # begins a comment
// #! and !# indicate the start and end of block comments
// @ declares a new macro
// @@ inserts the current macro in the name
// & declares a counter (for RHS value) or preincrements and inserts the counter (for LHS name)
// ^ inserts the counter (for LHS name) without incrementing

// Do *NOT* use this as a base for any kind of scripting language.
// This is very specifically written to parse the "key = value"
// lines of a config file and will not scale up well.\

// For parsing ints and floats
#include <stdlib.h>
#include <math.h>

// For strlen
#include <string.h>

// TODO: Handle errors better--right now, I'm relying on ASSERTs.

struct SToken
{
	enum ETokenType
	{
		ET_None,
		ET_Name,
		ET_Equals,
		ET_Bool,
		ET_Int,
		ET_Float,
		ET_String,
		ET_Context,
		ET_Macro,
		ET_Counter,
	};

	SToken();
	~SToken();

	ETokenType		m_TokenType;
	Array< char >	m_TokenString;

	static char*	m_TokenNames[];
};

char* SToken::m_TokenNames[] =
{
	"ET_None",
	"ET_Name",
	"ET_Equals",
	"ET_Bool",
	"ET_Int",
	"ET_Float",
	"ET_String",
	"ET_Context",
	"ET_Macro",
	"ET_Counter",
};

SToken::SToken()
:	m_TokenType( ET_None )
,	m_TokenString()
{
}

SToken::~SToken()
{
	m_TokenString.Clear();
}

bool IsNum( char c )
{
	return ( c >= '0' && c <= '9' );
}

bool IsHex( char c )
{
	return ( ( c >= '0' && c <= '9' ) || ( c >= 'a' || c <= 'f' ) || ( c >= 'A' || c <= 'F' ) );
}

bool IsBool( char c )
{
	return ( c == 't' || c == 'T' || c == 'f' || c== 'F' );
}

char GetHex( char c )
{
	switch( c )
	{
	case '0':
		return 0;
	case '1':
		return 1;
	case '2':
		return 2;
	case '3':
		return 3;
	case '4':
		return 4;
	case '5':
		return 5;
	case '6':
		return 6;
	case '7':
		return 7;
	case '8':
		return 8;
	case '9':
		return 9;
	case 'a':
	case 'A':
		return 10;
	case 'b':
	case 'B':
		return 11;
	case 'c':
	case 'C':
		return 12;
	case 'd':
	case 'D':
		return 13;
	case 'e':
	case 'E':
		return 14;
	case 'f':
	case 'F':
		return 15;
	default:
		return 0;
	}
}

// InnerParse is poorly named; it really just accepts any character not handled in the calling function
// and determines the token type for unknown right-hand tokens.
void InnerParse( const char c, char& StrMark, Array< char >& TokenString, const int LineCount, SToken::ETokenType& InOutTokenType, bool* OutClosedString = NULL )
{
	switch( InOutTokenType )
	{
	case SToken::ET_None:
		// Determine the type of the RHS from the first character
		// (Can change from int to float later, on finding a . or f)
		if( IsNum( c ) || c == '-' )
		{
			InOutTokenType = SToken::ET_Int;
			TokenString.PushBack( c );
		}
		else if( IsBool( c ) )
		{
			InOutTokenType = SToken::ET_Bool;
			TokenString.PushBack( c );
		}
		else if( c == '\"' || c == '\'' )
		{
			InOutTokenType = SToken::ET_String;
			StrMark = c;
		}
		else if( c == '.' )
		{
			InOutTokenType = SToken::ET_Float;
			TokenString.PushBack( c );
		}
		else
		{
			PRINTF( "Unexpected character %c in undetermined token at line %d\n", c, LineCount );
			WARNDESC( "Unexpected character in undetermined token" );
		}
		break;
	case SToken::ET_Name:
	case SToken::ET_Context:
	case SToken::ET_Macro:
		TokenString.PushBack( c );
		break;
	case SToken::ET_Bool:
		TokenString.PushBack( c );
		break;
	case SToken::ET_Int:
		if( c == '.' )
		{
			InOutTokenType = SToken::ET_Float;
		}
		else
		{
			ASSERT( IsNum( c ) );
		}
		TokenString.PushBack( c );
		break;
	case SToken::ET_Float:
		ASSERT( IsNum( c ) || c == 'f' );
		TokenString.PushBack( c );
		break;
	case SToken::ET_String:
		if( c == StrMark )
		{
			TokenString.PushBack( '\0' );
			if( OutClosedString )
			{
				*OutClosedString = true;
			}
		}
		else
		{
			TokenString.PushBack( c );
		}
		break;
	default:
		WARNDESC( "Unexpected token" );
		break;
	}
}

#define UTF8_BOM_0	0xef
#define UTF8_BOM_1	0xbb
#define UTF8_BOM_2	0xbf

void ConfigParser::Parse( const IDataStream& Stream )
{
	Array< SToken >	Tokens;
	SToken			Token;
	Token.m_TokenType						= SToken::ET_Name;
	char			StrMark					= 0;	// Stores the character (either ' or ") that opened a string
	SToken			MacroToken;
	List<int>		ArrayCounters;
	Array<char>		CounterCharArray;
	int				LineCount				= 1;

	for(;;)
	{
		char c = Stream.ReadInt8();

		// Skip the UTF-8 byte order mark if present.
		if( UTF8_BOM_0 == static_cast<byte>( c ) )
		{
			CHECK( UTF8_BOM_1 == static_cast<byte>( Stream.ReadInt8() ) );
			CHECK( UTF8_BOM_2 == static_cast<byte>( Stream.ReadInt8() ) );
			c = Stream.ReadInt8();
		}

		if( Stream.EOS() )
		{
			if( Token.m_TokenString.Empty() )
			{
				Token.m_TokenType = SToken::ET_None;
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s\n", SToken::m_TokenNames[ Token.m_TokenType ] );
			}
			else
			{
				Token.m_TokenString.PushBack( '\0' );
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
				Token.m_TokenString.Clear();
			}

			break;
		}

		if( c == '&' )
		{
			if( Token.m_TokenType == SToken::ET_Name )
			{
				// Increment the current counter and add it to the current name.
				ASSERT( ArrayCounters.Size() > 0 );
				List<int>::Iterator CounterIter = ArrayCounters.Back();
				( *CounterIter )++;
				SimpleString CounterString = SimpleString::PrintF( "%d", *CounterIter );
				CounterCharArray.Clear();
				CounterString.FillArray( CounterCharArray );
				Token.m_TokenString.Append( CounterCharArray );
			}
			else if( Token.m_TokenType == SToken::ET_None )
			{
				// Add a new counter
				// Push a counter token that will be replaced with the count int later.
				ArrayCounters.PushBack( -1 );
				Token.m_TokenType = SToken::ET_Counter;
			}
			else if( Token.m_TokenType == SToken::ET_String )
			{
				Token.m_TokenString.PushBack( c );
			}
			else
			{
				WARNDESC( "Unexpected character '&' in token." );
			}
		}
		else if( c == '^' )
		{
			if( Token.m_TokenType == SToken::ET_Name )
			{
				// Add the current counter to the current name.
				ASSERT( ArrayCounters.Size() > 0 );
				List<int>::Iterator CounterIter = ArrayCounters.Back();
				ASSERT( ( *CounterIter ) >= 0 );
				SimpleString CounterString = SimpleString::PrintF( "%d", *CounterIter );
				CounterCharArray.Clear();
				CounterString.FillArray( CounterCharArray );
				Token.m_TokenString.Append( CounterCharArray );
			}
			else if( Token.m_TokenType == SToken::ET_String )
			{
				Token.m_TokenString.PushBack( c );
			}
			else
			{
				WARNDESC( "Unexpected character '^' in token." );
			}
		}
		else if( c == ' ' || c == '\t' )
		{
			switch( Token.m_TokenType )
			{
			case SToken::ET_None:
			case SToken::ET_Equals:
				// Ignore whitespace
				break;
			case SToken::ET_Name:
			case SToken::ET_Context:
			case SToken::ET_Macro:
				if( Token.m_TokenString.Empty() )
				{
					// If the name is empty, ignore whitespace (before the name)
				}
				else
				{
					// Close current token, push it, and expect an equals

					// If we're closing a macro, save it as such
					if( Token.m_TokenType == SToken::ET_Macro )
					{
						MacroToken = Token;
					}

					Token.m_TokenString.PushBack( '\0' );
					Tokens.PushBack( Token );
					DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
					Token.m_TokenString.Clear();

					if( Token.m_TokenType == SToken::ET_Name )
					{
						Token.m_TokenType = SToken::ET_Equals;
					}
					else if( Token.m_TokenType == SToken::ET_Context || Token.m_TokenType == SToken::ET_Macro )
					{
						Token.m_TokenType = SToken::ET_Name;
					}
				}
				break;
			case SToken::ET_Bool:
			case SToken::ET_Int:
			case SToken::ET_Float:
				// Close current token, push it, and expect nothing
				Token.m_TokenString.PushBack( '\0' );
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
				Token.m_TokenString.Clear();

				Token.m_TokenType = SToken::ET_None;
				break;
			case SToken::ET_String:
				Token.m_TokenString.PushBack( c );
				break;
			default:
				WARNDESC( "Unexpected token" );
				break;
			}
		}
		else if( c == '=' )
		{
			switch( Token.m_TokenType )
			{
			case SToken::ET_Name:
				// Close current token, push it and an equals
				Token.m_TokenString.PushBack( '\0' );
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
				Token.m_TokenString.Clear();

				Token.m_TokenType = SToken::ET_Equals;
				DEBUGCATPRINTF( "Core", 2, "%s\n", SToken::m_TokenNames[ Token.m_TokenType ] );
				Tokens.PushBack( Token );

				Token.m_TokenType = SToken::ET_None;
				break;
			case SToken::ET_Equals:
				// Already expecting =, just push it
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s\n", SToken::m_TokenNames[ Token.m_TokenType ] );

				Token.m_TokenType = SToken::ET_None;
				break;
			case SToken::ET_String:
				Token.m_TokenString.PushBack( c );
				break;
			default:
				WARNDESC( "Unexpected token" );
				break;
			}
		}
		else if( c == '#' )
		{
			// # starts a comment

			// Allow # inside a string
			if( Token.m_TokenType == SToken::ET_String )
			{
				Token.m_TokenString.PushBack( c );
			}
			else
			{
				c = Stream.ReadInt8();
				
				if( c == '!' )	// #! and !# indicate the start and end of block comments
				{
					while( !Stream.EOS() )
					{
						if( Stream.ReadInt8() == '!' && Stream.ReadInt8() == '#' )
						{
							break;
						}
					}
				}
				else
				{
					// Read to end of line
					while( c != '\n' && c!= '\v' && !Stream.EOS() )	// Vertical tab stupidity again
					{
						c = Stream.ReadInt8();
					}

					// Change the context because we're on a new line
					Token.m_TokenType = SToken::ET_Name;

					++LineCount;
				}
			}
		}
		else if( c == '\\' && Token.m_TokenType == SToken::ET_String )
		{
			// Escape sequence, intended to insert linebreaks (and maybe other things in the future)
			// Config string escape sequences are not the same as C++ escape sequences. They can be
			// \\, \n, \?, \", or \xx (where x are hex digits, to specify any character by hex).

			char next = Stream.ReadInt8();
			if( next == 'n' )
			{
				Token.m_TokenString.PushBack( '\n' );
			}
			else if( next == '\"' )
			{
				Token.m_TokenString.PushBack( '\"' );
			}
			else if( next == '\\' )
			{
				Token.m_TokenString.PushBack( '\\' );
			}
			else if( next == 'x' )
			{
				char Hex = 0;
				for( uint HexIndex = 0; HexIndex < 2; ++HexIndex )
				{
					next = Stream.ReadInt8();
					ASSERT( IsHex( next ) );
					Hex = ( Hex << 4 ) | GetHex( next );
				}
				Token.m_TokenString.PushBack( Hex );
			}
			else if( next == 'u' )
			{
				// First, extract a unicode code point (e.g. \u00d7 for U+00D7)
				// NOTE: This only support the first Unicode plane, and is strict about
				// using four characters, so \ud7 is not a valid substitute for \u00d7.
				unicode_t CodePoint = 0;
				for( uint UnicodeIndex = 0; UnicodeIndex < 4; ++UnicodeIndex )
				{
					next = Stream.ReadInt8();
					ASSERT( IsHex( next ) );
					CodePoint = ( CodePoint << 4 ) | GetHex( next );
				}

				// Then convert the two-byte code point to UTF-8.
				Array<unicode_t> CodePointArray;
				CodePointArray.PushBack( CodePoint );
				const SimpleString UTF8String = SimpleString::SetUTF8( CodePointArray );

				for( uint CharIndex = 0; CharIndex < UTF8String.Length(); ++CharIndex )
				{
					const char NextChar = UTF8String.GetChar( CharIndex );
					Token.m_TokenString.PushBack( NextChar );
				}
			}
			else
			{
				PRINTF( "Unrecognized escape sequence \\%c at line %d\n", next, LineCount );
				WARNDESC( "Unrecognized escape sequence" );
			}
		}
		else if( c == 0x0d )
		{
			// DOS linebreak is 0D 0A, so ignore and expect \n to follow
		}
		else if( c == '\0' )
		{
			// Don't know how these are getting in either, but ignore them
		}
		else if( c == '\n' || c == '\v' )
		{
			if( Token.m_TokenType == SToken::ET_Macro )
			{
				MacroToken = Token;
			}

			// Dunno how vertical tabs are getting in, but treat them as linebreaks
			if( Token.m_TokenString.Empty() )
			{
				if( Token.m_TokenType != SToken::ET_Counter )
				{
					Token.m_TokenType = SToken::ET_None;
				}
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s\n", SToken::m_TokenNames[ Token.m_TokenType ] );

				Token.m_TokenType = SToken::ET_Name;
			}
			else
			{
				Token.m_TokenString.PushBack( '\0' );
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
				Token.m_TokenString.Clear();

				Token.m_TokenType = SToken::ET_Name;
			}

			++LineCount;
		}
		else if( c == '[' )
		{
			if( Token.m_TokenType == SToken::ET_String )
			{
				Token.m_TokenString.PushBack( c );
			}
			else
			{
				// We should only ever open a context when we're expecting a name
				ASSERT( Token.m_TokenType == SToken::ET_Name );
				Token.m_TokenType = SToken::ET_Context;

				// Opening a new context, clear the macro token.
				MacroToken = SToken();
			}
		}
		else if( c == ']' )
		{
			// If we've already closed the context, ignore; else, push token
			if( Token.m_TokenType == SToken::ET_String )
			{
				Token.m_TokenString.PushBack( c );
			}
			else
			{
				ASSERT( Token.m_TokenType == SToken::ET_Context );
				Token.m_TokenString.PushBack( '\0' );
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
				Token.m_TokenString.Clear();
			}
		}
		else if( c == '@' )
		{
			if( Token.m_TokenType == SToken::ET_String )
			{
				Token.m_TokenString.PushBack( c );
			}
			else
			{
				// We should only ever declare or insert a macro when we're expecting a name
				ASSERT( Token.m_TokenType == SToken::ET_Name );
				c = Stream.ReadInt8();
				if( c == '@' )
				{
					// @@... means we're inserting the current macro into a name

					// Make sure there is a current macro. If this fails, a macro probably
					// wasn't opened in the current context.
					ASSERT( MacroToken.m_TokenString.Size() > 0 );

					const uint MacroLength = MacroToken.m_TokenString.Size();
					for( uint MacroIndex = 0; MacroIndex < MacroLength; ++MacroIndex )
					{
						Token.m_TokenString.PushBack( MacroToken.m_TokenString[ MacroIndex ] );
					}
				}
				else
				{
					// @... means we're declaring a new macro
					Token.m_TokenType = SToken::ET_Macro;
					if( c == ' ' || c == '\t' )
					{
						// Ignore whitespace at the front of macro
					}
					else
					{
						Token.m_TokenString.PushBack( c );
					}
				}
			}
		}
		else
		{
			bool ClosedString = false;
			InnerParse( c, StrMark, Token.m_TokenString, LineCount, Token.m_TokenType, &ClosedString );
			if( ClosedString )
			{
				Tokens.PushBack( Token );
				DEBUGCATPRINTF( "Core", 2, "%s: %s\n", SToken::m_TokenNames[ Token.m_TokenType ], Token.m_TokenString.GetData() );
				Token.m_TokenString.Clear();
				Token.m_TokenType = SToken::ET_None;
			}
		}
	}

	SimpleString Context = "";

	// Tokens are made, now create config vars
	for( uint i = 0; i < Tokens.Size(); ++i )
	{
		SToken& NameToken = Tokens[i];
		SimpleString Name = "";
		const char* ValueString = NULL;
		if( NameToken.m_TokenType == SToken::ET_Name )
		{
			Name = NameToken.m_TokenString.GetData();
			ASSERT( Tokens[ i + 1 ].m_TokenType == SToken::ET_Equals );

			SToken& ValueToken = Tokens[ i + 2 ];
			ValueString = ValueToken.m_TokenString.GetData();

			if( Context != "" )
			{
				CATPRINTF( "Core", 2, "%s:", Context.CStr() );
			}
			CATPRINTF( "Core", 2, "%s: %s: %s\n", Name.CStr(), SToken::m_TokenNames[ ValueToken.m_TokenType ], ValueString );

			switch( ValueToken.m_TokenType )
			{
			case SToken::ET_Bool:
				{
					// Just use the first character to determine truth
					bool Value = false;
					char first = ValueString[0];
					if( first == 't' || first == 'T' )
					{
						Value = true;
					}
					ConfigManager::SetBool( Name, Value, Context );
				}
				break;
			case SToken::ET_Int:
				{
					int Value = atoi( ValueString );
					ConfigManager::SetInt( Name, Value, Context );
				}
				break;
			case SToken::ET_Counter:
				{
					List<int>::Iterator NextCounterIter = ArrayCounters.Front();
					( *NextCounterIter )++;	// Add one to the last value we incremented, and that's the total for this array
					ConfigManager::SetInt( Name, *NextCounterIter, Context );
					ArrayCounters.PopFront();
				}
				break;
			case SToken::ET_Float:
				{
					float Value = (float)atof( ValueString );
					ConfigManager::SetFloat( Name, Value, Context );
				}
				break;
			case SToken::ET_String:
				{
					// Make a permanent copy of the string
					uint Length = (uint)strlen( ValueString );
					char* pString = new char[ Length + 1];
					memcpy_s( pString, Length + 1, ValueString, Length );
					pString[ Length ] = '\0';
					StringManager::AddString( StringManager::ESL_Permanent, pString );

					ConfigManager::SetString( Name, pString, Context );
				}

				break;
			default:
				WARNDESC( "Unexpected token" );
				break;
			}

			i += 2;
		}
		else if( NameToken.m_TokenType == SToken::ET_Context )
		{
			Context = NameToken.m_TokenString.GetData();
			//CATPRINTF( "Core", 2, "Pushed context %s\n", Context.CStr() );
		}
		else
		{
			DEBUGCATPRINTF( "Core", 2, "Skipped unexpected token %s (expected ET_Name)\n", SToken::m_TokenNames[ NameToken.m_TokenType ] );
		}
	}

	// Clean up
	for( uint i = 0; i < Tokens.Size(); ++i )
	{
		Tokens[i].m_TokenString.Clear();
	}
	Tokens.Clear();
}

// NOTE: This is a lot of duplicated code from the value parsing parts
// of Parse(). Could do to clean this up and use common functions.
void ConfigParser::ParseTiny( const IDataStream& Stream )
{
	Array< char >		FirstPart;	// Could be name or context
	Array< char >		SecondPart;	// Name if first part is context
	Array< char >		ValuePart;
	SToken::ETokenType	Type = SToken::ET_None;
	char				QuoteType = '\"';
	bool				StringClosed = false;

	enum EParseState
	{
		EPS_FirstPart,
		EPS_SecondPart,
		EPS_ValuePart,
	} ParseState = EPS_FirstPart;

	while( !Stream.EOS() )
	{
		char c = Stream.ReadInt8();

		if( ParseState == EPS_FirstPart )
		{
			if( c == ' ' || c == '\t' || c == '\n' || c == '\0' )
			{
				// Ignore whitespace
				continue;
			}
			else if( c == ':' )
			{
				FirstPart.PushBack( '\0' );
				ParseState = EPS_SecondPart;
				continue;
			}
			else if( c == '=' )
			{
				FirstPart.PushBack( '\0' );
				ParseState = EPS_ValuePart;
				continue;
			}
			else
			{
				FirstPart.PushBack( c );
			}
		}
		else if( ParseState == EPS_SecondPart )
		{
			if( c == ' ' || c == '\t' || c == '\n' || c == '\0' )
			{
				// Ignore whitespace
				continue;
			}
			else if( c == '=' )
			{
				SecondPart.PushBack( '\0' );
				ParseState = EPS_ValuePart;
				continue;
			}
			else
			{
				SecondPart.PushBack( c );
			}
		}
		else if( ParseState == EPS_ValuePart )
		{
			if( ( ( Type != SToken::ET_String || StringClosed ) && ( c == ' ' || c == '\t' ) ) || c == '\n' || c == '\0' )
			{
				// Parse the value and set the config var
				ValuePart.PushBack( '\0' );

				SimpleString Name = ( SecondPart.Size() > 1 ) ? SecondPart.GetData() : FirstPart.GetData();
				SimpleString Context = ( SecondPart.Size() > 1 ) ? FirstPart.GetData() : "";

				switch( Type )
				{
				case SToken::ET_Bool:
					{
						// Just use the first character to determine truth
						bool Value = false;
						char first = ValuePart[0];
						if( first == 't' || first == 'T' )
						{
							Value = true;
						}
						ConfigManager::SetBool( Name, Value, Context );
					}
					break;
				case SToken::ET_Int:
					{
						int Value = atoi( ValuePart.GetData() );
						ConfigManager::SetInt( Name, Value, Context );
					}
					break;
				case SToken::ET_Float:
					{
						float Value = (float)atof( ValuePart.GetData() );
						ConfigManager::SetFloat( Name, Value, Context );
					}
					break;
				case SToken::ET_String:
					{
						// Make a permanent copy of the string
						uint Length = (uint)strlen( ValuePart.GetData() );
						char* pString = new char[ Length + 1];
						memcpy_s( pString, Length + 1, ValuePart.GetData(), Length );
						pString[ Length ] = '\0';
						StringManager::AddString( StringManager::ESL_Permanent, pString );

						ConfigManager::SetString( Name, pString, Context );
					}

					break;
				default:
					WARNDESC( "Unexpected token" );
					break;
				}

				// Prepare for next variable
				FirstPart.Clear();
				SecondPart.Clear();
				ValuePart.Clear();
				Type = SToken::ET_None;
				ParseState = EPS_FirstPart;
				StringClosed = false;
				continue;
			}
			else
			{
				InnerParse( c, QuoteType, ValuePart, 0, Type, &StringClosed );
			}
		}
	}
}

// It might be useful to have some arithmetic parsing for more complex expressions,
// but eventually that would just become writing a whole little language...
// NOTE: This is very error-prone if the format isn't exactly "A OP B", so
// don't use this for anything that the user can touch.
bool ConfigParser::EvaluateConditional( const IDataStream& Stream )
{
	// Parse three tokens: a left side, an operator, and a right side
	// Valid expressions:
	// bool b-op bool
	// int n-op int
	// int n-op float
	// float n-op float
	// float n-op int
	// string s-op string
	// b-op: == !=
	// n-op: < <= > >= == !=
	// s-op: == != ~= ~!= (case-insensitive)

	// Use same rules as above to determine the type of each value
	// This would be a good place to refactor to reuse some code

	SToken::ETokenType LeftSideType = SToken::ET_None;
	SToken::ETokenType RightSideType = SToken::ET_None;

	Array< char > LeftSideString;
	Array< char > OperatorString;
	Array< char > RightSideString;

	char StrMark = '\"';

	// Parse left side
	for(;;)
	{
		char c = Stream.ReadInt8();
		if( c == ' ' || Stream.EOS() )
		{
			LeftSideString.PushBack( '\0' );
			break;
		}
		else
		{
			InnerParse( c, StrMark, LeftSideString, 0, LeftSideType );
		}
	}

	// Parse operator
	for(;;)
	{
		char c = Stream.ReadInt8();
		if( c == ' ' || Stream.EOS() )
		{
			OperatorString.PushBack( '\0' );
			break;
		}
		OperatorString.PushBack( c );
	}

	// Parse right side
	for(;;)
	{
		char c = Stream.ReadInt8();
		if( c == '\0' || c == ' ' || Stream.EOS() )
		{
			RightSideString.PushBack( '\0' );
			break;
		}
		else
		{
			InnerParse( c, StrMark, RightSideString, 0, RightSideType );
		}
	}

	// Evaluate
	if( LeftSideType == RightSideType ||
		( LeftSideType == SToken::ET_Int && RightSideType == SToken::ET_Float ) ||
		( LeftSideType == SToken::ET_Float && RightSideType == SToken::ET_Int ) )
	{
		SimpleString LeftSide = LeftSideString.GetData();
		SimpleString RightSide = RightSideString.GetData();
		SimpleString Operator = OperatorString.GetData();

		if( LeftSideType == SToken::ET_Bool )
		{
			bool LeftSideBool = LeftSide.AsBool();
			bool RightSideBool = RightSide.AsBool();

			if( Operator == "==" )
			{
				return ( LeftSideBool == RightSideBool );
			}
			else if( Operator == "!=" )
			{
				return ( LeftSideBool != RightSideBool );
			}
			else
			{
				DEBUGWARNDESC( "Unknown operator in conditional expression" );
				return false;
			}
		}
		else if( LeftSideType == SToken::ET_Int )
		{
			int LeftSideInt = LeftSide.AsInt();
			int RightSideInt = ( RightSideType == SToken::ET_Int ) ? RightSide.AsInt() : (int)RightSide.AsFloat();

			if( Operator == "==" )
			{
				return ( LeftSideInt == RightSideInt );
			}
			else if( Operator == "!=" )
			{
				return ( LeftSideInt != RightSideInt );
			}
			else if( Operator == "<" )
			{
				return ( LeftSideInt < RightSideInt );
			}
			else if( Operator == "<=" )
			{
				return ( LeftSideInt <= RightSideInt );
			}
			else if( Operator == ">" )
			{
				return ( LeftSideInt > RightSideInt );
			}
			else if( Operator == ">=" )
			{
				return ( LeftSideInt >= RightSideInt );
			}
			else
			{
				DEBUGWARNDESC( "Unknown operator in conditional expression" );
				return false;
			}
		}
		else if( LeftSideType == SToken::ET_Float )
		{
			float LeftSideFloat = LeftSide.AsFloat();
			float RightSideFloat = ( RightSideType == SToken::ET_Float ) ? RightSide.AsFloat() : (float)RightSide.AsInt();

			if( Operator == "==" )
			{
				return ( LeftSideFloat == RightSideFloat );
			}
			else if( Operator == "!=" )
			{
				return ( LeftSideFloat != RightSideFloat );
			}
			else if( Operator == "<" )
			{
				return ( LeftSideFloat < RightSideFloat );
			}
			else if( Operator == "<=" )
			{
				return ( LeftSideFloat <= RightSideFloat );
			}
			else if( Operator == ">" )
			{
				return ( LeftSideFloat > RightSideFloat );
			}
			else if( Operator == ">=" )
			{
				return ( LeftSideFloat >= RightSideFloat );
			}
			else
			{
				DEBUGWARNDESC( "Unknown operator in conditional expression" );
				return false;
			}
		}
		else if( LeftSideType == SToken::ET_String )
		{
			if( Operator == "==" )
			{
				return ( LeftSide == RightSide );
			}
			else if( Operator == "!=" )
			{
				return ( LeftSide != RightSide );
			}
			else if( Operator == "~=" )
			{
				return ( LeftSide.StrICmp( RightSide ) );
			}
			else if( Operator == "~!=" )
			{
				return ( !LeftSide.StrICmp( RightSide ) );
			}
			else
			{
				DEBUGWARNDESC( "Unknown operator in conditional expression" );
				return false;
			}
		}
		else
		{
			DEBUGWARNDESC( "Unknown types in conditional expression" );
			return false;
		}
	}
	else
	{
		DEBUGWARNDESC( "Mismatched types in conditional expression" );
		return false;
	}
}