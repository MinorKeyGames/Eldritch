#include "core.h"
#include "simplestring.h"
#include "filestream.h"
#include "allocator.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>

#define UTF8_1BYTEMASK		0x80
#define UTF8_1BYTEMARKER	0x00
#define UTF8_2BYTEMASKHIGH	0xe0
#define UTF8_2BYTEMARKER	0xc0
#define UTF8_2BYTEMASKLOW	0x1f
#define UTF8_3BYTEMASKHIGH	0xf0
#define UTF8_3BYTEMARKER	0xe0
#define UTF8_3BYTEMASKLOW	0x0f
#define UTF8_4BYTEMASKHIGH	0xf8
#define UTF8_4BYTEMARKER	0xf0
#define UTF8_4BYTEMASKLOW	0x07
#define UTF8_CBYTEMASKHIGH	0xc0
#define UTF8_CBYTEMARKER	0x80
#define UTF8_CBYTEMASKLOW	0x3f
#define UTF8_CBYTESHIFT		6

Allocator SimpleString::m_AllocatorSmall( "SimpleString/Small" );
Allocator SimpleString::m_AllocatorLarge( "SimpleString/Large" );
bool SimpleString::m_UsingAllocator = false;

/*explicit*/ SimpleString::SimpleString( bool NoInit )
:	m_String( NULL )
,	m_Length( 0 )
{
	Unused( NoInit );
}

SimpleString::SimpleString( const char* pString /*=""*/ )
:	m_String( NULL )
,	m_Length( 0 )
{
	Initialize( pString );
}

SimpleString::SimpleString( const char* pNonTerminatedString, uint Length )
:	m_String( NULL )
,	m_Length( 0 )
{
	InitializeNonTerminated( pNonTerminatedString, Length );
}

SimpleString::SimpleString( const SimpleString& String )
:	m_String( NULL )
,	m_Length( 0 )
{
	Initialize( String.m_String );
}

SimpleString::SimpleString( const Array<char>& String )
:	m_String( NULL )
,	m_Length( 0 )
{
	Initialize( String.GetData() );
}

SimpleString::~SimpleString()
{
	SafeDeleteArray( m_String );
}

SimpleString& SimpleString::operator=( const SimpleString& String )
{
	if( String.m_String != m_String )
	{
		SafeDeleteArray( m_String );
		Initialize( String.m_String );
	}
	return *this;
}

SimpleString& SimpleString::operator=( const char* pString )
{
	if( pString != m_String )
	{
		SafeDeleteArray( m_String );
		Initialize( pString );
	}
	return *this;
}

SimpleString& SimpleString::operator=( const Array< char >& String )
{
	SafeDeleteArray( m_String );
	Initialize( String.GetData() );
	return *this;
}

SimpleString SimpleString::operator+( const SimpleString& String ) const
{
	uint NewLength = m_Length + String.m_Length;
	char* NewString = Allocate( NewLength + 1 );

	memcpy( NewString, m_String, m_Length );
	memcpy( NewString + m_Length, String.m_String, String.m_Length );
	NewString[ NewLength ] = '\0';

	SimpleString RetVal( NewString );
	SafeDeleteArray( NewString );

	return RetVal;
}

SimpleString& SimpleString::operator+=( const SimpleString& String )
{
	uint NewLength = m_Length + String.m_Length;
	char* NewString = Allocate( NewLength + 1 );

	memcpy( NewString, m_String, m_Length );
	memcpy( NewString + m_Length, String.m_String, String.m_Length );
	NewString[ NewLength ] = '\0';

	SafeDeleteArray( m_String );
	Initialize( NewString );
	SafeDeleteArray( NewString );

	return *this;
}

bool SimpleString::operator==( const SimpleString& String ) const
{
	return ( 0 == strcmp( m_String, String.m_String ) );
}

bool SimpleString::operator!=( const SimpleString& String ) const
{
	return ( 0 != strcmp( m_String, String.m_String ) );
}

bool SimpleString::operator<( const SimpleString& String ) const
{
	return ( 0 > strcmp( m_String, String.m_String ) );
}

bool SimpleString::StrICmp( const SimpleString& String ) const
{
#if BUILD_WINDOWS
	return ( 0 == _stricmp( m_String, String.m_String ) );
#else
	return ( 0 == strcasecmp( m_String, String.m_String ) );
#endif
}

const char* SimpleString::CStr() const
{
	return m_String;
}

char* SimpleString::MutableCStr()
{
	return m_String;
}

// Doesn't include the terminating null
uint SimpleString::Length() const
{
	return m_Length;
}

// Cheap trick, might be useful for console commands
// Returns true iff first character is 't' or 'T'
bool SimpleString::AsBool() const
{
	if( *m_String == 't' || *m_String == 'T' )
	{
		return true;
	}
	else
	{
		return false;
	}
}

int SimpleString::AsInt() const
{
	return atoi( m_String );
}

float SimpleString::AsFloat() const
{
	return (float)atof( m_String );
}

void SimpleString::FillArray( Array<char>& OutArray, bool WithNull /*= false*/ ) const
{
	const uint Length = WithNull ? m_Length + 1 : m_Length;
	OutArray.Clear();
	OutArray.Resize( Length );
	memcpy_s( OutArray.GetData(), Length, m_String, Length );
}

void SimpleString::Initialize( const char* pString )
{
	DEVASSERTDESC( pString, "SimpleStrings must be initialized--use \"\" instead of NULL" );	// SimpleStrings should always represent something (same as std::string)
	DEVASSERT( !m_String );

	// Safeguard against this error in final builds.
	if( !pString )
	{
		pString = "";
	}

	if( pString )
	{
		m_Length = ( uint )strlen( pString );
		m_String = Allocate( m_Length + 1 );
		strcpy_s( m_String, m_Length + 1, pString );
	}
}

void SimpleString::InitializeNonTerminated( const char* pString, uint Length )
{
	DEVASSERTDESC( pString, "SimpleStrings must be initialized--use \"\" instead of NULL" );	// SimpleStrings should always represent something (same as std::string)
	DEVASSERT( !m_String );
	if( pString )
	{
		m_Length = Length;
		m_String = Allocate( m_Length + 1 );
		memcpy_s( m_String, m_Length + 1, pString, m_Length );
		m_String[ m_Length ] = '\0';
	}
}

bool SimpleString::Contains( const SimpleString& String ) const
{
	return ( strstr( m_String, String.m_String ) != NULL );
}

bool SimpleString::BeginsWith( const SimpleString& String ) const
{
	return ( strstr( m_String, String.m_String ) == m_String );
}

void SimpleString::Replace( const char Char, const char ReplaceChar ) const
{
	for( uint CharIndex = 0; CharIndex < m_Length; ++CharIndex )
	{
		char& IterChar = m_String[ CharIndex ];
		if( IterChar == Char )
		{
			IterChar = ReplaceChar;
		}
	}
}

SimpleString SimpleString::Replace( const char* const Find, const char* const Replace ) const
{
	DEVASSERT( Find );
	DEVASSERT( Replace );

	Array<char> NewStringBuffer;
	NewStringBuffer.Resize( m_Length + 1 );
	memcpy( NewStringBuffer.GetData(), m_String, m_Length + 1	);

	const size_t FindLength		= strlen( Find );
	const size_t ReplaceLength	= strlen( Replace );
	const size_t Difference		= ReplaceLength - FindLength;

	for( size_t Iterator = 0; Iterator < NewStringBuffer.Size(); )
	{
		const char* const		SubStr	= strstr( NewStringBuffer.GetData() + Iterator, Find );
		if( SubStr )
		{
			const char* const	Base	= NewStringBuffer.GetData();
			const size_t		Offset	= SubStr - Base;

			for( size_t DifferenceIndex = 0; DifferenceIndex < Difference; ++DifferenceIndex )
			{
				NewStringBuffer.Insert( 0, static_cast<uint>( Offset ) );
			}

			memcpy( NewStringBuffer.GetData() + Offset, Replace, ReplaceLength );

			Iterator += ReplaceLength;
		}
		else
		{
			break;
		}
	}

	return SimpleString( NewStringBuffer );
}

void SimpleString::SplitFind( const char Delimiter, SimpleString& Left, SimpleString& Right ) const
{
	char Find[2];
	Find[0] = Delimiter;
	Find[1] = '\0';

	char* const SubStr = strstr( m_String, Find );
	if( SubStr )
	{
		*SubStr	= '\0';
		Left	= *this;
		*SubStr	= Delimiter;
		Right	= SubStr;
	}
	else
	{
		Left = *this;
		Right = "";
	}
}

void SimpleString::Split( uint Index, SimpleString& Left, SimpleString& Right ) const
{
	if( Index < m_Length )
	{
		char Temp			= m_String[ Index ];
		m_String[ Index ]	= '\0';
		Left				= *this;
		m_String[ Index ]	= Temp;
		Right				= m_String + Index;
	}
	else
	{
		Left = *this;
		Right = "";
	}
}

uint SimpleString::Find( const char* Find ) const
{
	const char* const SubStr = strstr( m_String, Find );
	if( SubStr )
	{
		return static_cast<uint>( SubStr - m_String );
	}
	else
	{
		return m_Length;
	}
}

SimpleString SimpleString::EscapeSequenceEncode() const
{
	SimpleString RetVal = *this;

	RetVal = RetVal.Replace( "\\", "\\\\" );
	RetVal = RetVal.Replace( "\"", "\\\"" );
	RetVal = RetVal.Replace( "\n", "\\n" );

	return RetVal;
}

// Hurgh, this would look nicer if strings used Arrays so I could more easily modify them in place.
SimpleString SimpleString::URLEncode() const
{
	SimpleString RetVal = *this;

	// Encoding character
	RetVal = RetVal.Replace( "%", "%25" );

	// Reserved characters
	RetVal = RetVal.Replace( "!", "%21" );
	RetVal = RetVal.Replace( "#", "%23" );
	RetVal = RetVal.Replace( "$", "%24" );
	RetVal = RetVal.Replace( "&", "%26" );
	RetVal = RetVal.Replace( "'", "%27" );
	RetVal = RetVal.Replace( "(", "%28" );
	RetVal = RetVal.Replace( ")", "%29" );
	RetVal = RetVal.Replace( "*", "%2A" );
	RetVal = RetVal.Replace( "+", "%2B" );
	RetVal = RetVal.Replace( ",", "%2C" );
	RetVal = RetVal.Replace( "/", "%2F" );
	RetVal = RetVal.Replace( ":", "%3A" );
	RetVal = RetVal.Replace( ";", "%3B" );
	RetVal = RetVal.Replace( "=", "%3D" );
	RetVal = RetVal.Replace( "?", "%3F" );
	RetVal = RetVal.Replace( "@", "%40" );
	RetVal = RetVal.Replace( "[", "%5B" );
	RetVal = RetVal.Replace( "]", "%5D" );

	// Other common characters
	RetVal = RetVal.Replace( " ", "%20" );
	RetVal = RetVal.Replace( "\"", "%22" );
	RetVal = RetVal.Replace( "-", "%2D" );
	RetVal = RetVal.Replace( ".", "%2E" );
	RetVal = RetVal.Replace( "<", "%3C" );
	RetVal = RetVal.Replace( ">", "%3E" );
	RetVal = RetVal.Replace( "\\", "%5C" );
	RetVal = RetVal.Replace( "^", "%5E" );
	RetVal = RetVal.Replace( "_", "%5F" );
	RetVal = RetVal.Replace( "`", "%60" );
	RetVal = RetVal.Replace( "{", "%7B" );
	RetVal = RetVal.Replace( "|", "%7C" );
	RetVal = RetVal.Replace( "}", "%7D" );
	RetVal = RetVal.Replace( "~", "%7E" );

	return RetVal;
}

SimpleString SimpleString::URLEncodeUTF8() const
{
	// First, encode all the usual stuff...
	const SimpleString BasicEncoded = URLEncode();

	Array<char> EncodedArray;
	BasicEncoded.FillArray( EncodedArray, true );

#define PERCENT_ENCODE_CHAR														\
	{																			\
		const char			c				= EncodedArray[ CharIter ];			\
		const uint8			UnsignedChar	= static_cast<uint8>( c );			\
		const SimpleString	Encoded			= PrintF( "%%%02X", UnsignedChar );	\
		const char* const	pEncoded		= Encoded.CStr();					\
		EncodedArray[ CharIter ] = *pEncoded;									\
		EncodedArray.Insert( *( pEncoded + 1 ), CharIter + 1 );					\
		EncodedArray.Insert( *( pEncoded + 2 ), CharIter + 2 );					\
		CharIter += 3;															\
	}

	for( uint CharIter = 0; CharIter < EncodedArray.Size(); )
	{
		const char c = EncodedArray[ CharIter ];
		if( ( c & UTF8_2BYTEMASKHIGH ) == UTF8_2BYTEMARKER )
		{
			// Encode c and following 2 byte
			PERCENT_ENCODE_CHAR;
			PERCENT_ENCODE_CHAR;
		}
		else if( ( c & UTF8_3BYTEMASKHIGH ) == UTF8_3BYTEMARKER )
		{
			// Encode c and following 2 bytes
			PERCENT_ENCODE_CHAR;
			PERCENT_ENCODE_CHAR;
			PERCENT_ENCODE_CHAR;
		}
		else if( ( c & UTF8_4BYTEMASKHIGH ) == UTF8_4BYTEMARKER )
		{
			// Encode c and following 3 bytes
			PERCENT_ENCODE_CHAR;
			PERCENT_ENCODE_CHAR;
			PERCENT_ENCODE_CHAR;
			PERCENT_ENCODE_CHAR;
		}
		else
		{
			++CharIter;
		}
	}

	SimpleString RetVal = EncodedArray;
	return RetVal;
}

SimpleString SimpleString::ToLower() const
{
	SimpleString LowerString = m_String;

	const char LowerOffset = 'a' - 'A';
	for( char* pc = LowerString.m_String; *pc; ++pc )
	{
		char& c = *pc;
		if( c >= 'A' && c <= 'Z' )
		{
			c += LowerOffset;
		}
	}

	return LowerString;
}

/*static*/ SimpleString SimpleString::PrintF( const char* FormatString, ... )
{
	va_list Args;
	va_start( Args, FormatString );

	int Length = VSPRINTF_COUNT( FormatString, Args ) + 1;
	char* pBuffer = Allocate( Length );
	VSPRINTF( pBuffer, Length, FormatString, Args );

	SimpleString RetVal( pBuffer );
	SafeDeleteArray( pBuffer );

	return RetVal;
}

char* SimpleString::Allocate( uint Size )
{
	if( m_UsingAllocator )
	{
		return new( GetAllocator( Size ) ) char[ Size ];
	}
	else
	{
		return new char[ Size ];
	}
}

Allocator& SimpleString::GetAllocator( uint Size )
{
	if( Size <= 8 )
	{
		return m_AllocatorSmall;
	}
	else
	{
		return m_AllocatorLarge;
	}
}

/*static*/ void SimpleString::InitializeAllocator( uint Size )
{
	m_AllocatorSmall.Initialize( Size >> 1 );
	m_AllocatorLarge.Initialize( Size >> 1 );

	m_UsingAllocator = true;
}

/*static*/ void SimpleString::ShutDownAllocator()
{
#if BUILD_DEBUG
	m_AllocatorSmall.Report( FileStream( "memory_exit_report.txt", FileStream::EFM_Append ) );
	m_AllocatorLarge.Report( FileStream( "memory_exit_report.txt", FileStream::EFM_Append ) );
#endif

	DEBUGASSERT( m_AllocatorSmall.CheckForLeaks() );
	DEBUGASSERT( m_AllocatorLarge.CheckForLeaks() );

	m_AllocatorSmall.ShutDown();
	m_AllocatorLarge.ShutDown();

	m_UsingAllocator = false;
}

/*static*/ void SimpleString::ReportAllocator( const SimpleString& Filename )
{
	Unused( Filename );
#if BUILD_DEBUG
	m_AllocatorSmall.Report( FileStream( Filename.CStr(), FileStream::EFM_Append ) );
	m_AllocatorLarge.Report( FileStream( Filename.CStr(), FileStream::EFM_Append ) );
#endif
}

void SimpleString::UTF8ToUnicode( Array<unicode_t>& OutUnicode ) const
{
	XTRACE_FUNCTION;
	PROFILE_FUNCTION;

	unicode_t	CodePoint	= 0;
	byte		NextByte	= 0;

#define UTF8_GETCONTINUATIONBYTE											\
	NextByte = static_cast<byte>( m_String[ ++ByteIndex ] );				\
	DEBUGASSERT( ( NextByte & UTF8_CBYTEMASKHIGH ) == UTF8_CBYTEMARKER );	\
	CodePoint <<= UTF8_CBYTESHIFT;											\
	CodePoint |= ( NextByte & UTF8_CBYTEMASKLOW )

	for( uint ByteIndex = 0; ByteIndex < m_Length; ++ByteIndex )
	{
		NextByte = static_cast<byte>( m_String[ ByteIndex ] );

		if( ( NextByte & UTF8_1BYTEMASK ) == UTF8_1BYTEMARKER )
		{
			// This is a code point in the range U+0000-U+007F and needs no translation.
			CodePoint = NextByte;
		}
		else if( ( NextByte & UTF8_2BYTEMASKHIGH ) == UTF8_2BYTEMARKER )
		{
			// This is a 2-byte code point in the range U+0080-U+07FF.
			CodePoint = ( NextByte & UTF8_2BYTEMASKLOW );
			UTF8_GETCONTINUATIONBYTE;
		}
		else if( ( NextByte & UTF8_3BYTEMASKHIGH ) == UTF8_3BYTEMARKER )
		{
			// This is a 3-byte code point in the range U+0800-U+FFFF.
			CodePoint = ( NextByte & UTF8_3BYTEMASKLOW );
			UTF8_GETCONTINUATIONBYTE;
			UTF8_GETCONTINUATIONBYTE;
		}
		else if( ( NextByte & UTF8_4BYTEMASKHIGH ) == UTF8_4BYTEMARKER )
		{
			// This is a 4-byte code point in the range U+10000-U+1FFFFF (constrained to U+10FFFF by definition).
			CodePoint = ( NextByte & UTF8_4BYTEMASKLOW );
			UTF8_GETCONTINUATIONBYTE;
			UTF8_GETCONTINUATIONBYTE;
			UTF8_GETCONTINUATIONBYTE;
		}
		else
		{
			// This is not a valid UTF-8 stream.
			WARN;
		}

		OutUnicode.PushBack( CodePoint );
	}

#undef UTF8_GETCONTINUATIONBYTE
}

#define UTF8_1BYTERANGE	0x007f
#define UTF8_2BYTERANGE	0x07ff
#define UTF8_3BYTERANGE	0xffff
/*static*/ SimpleString SimpleString::SetUTF8( const Array<unicode_t>& Unicode )
{
	Array<char> UTF8Array;
	UTF8Array.Reserve( Unicode.Size() );	// We know we'll have at least as many chars as code points

	FOR_EACH_ARRAY( CodePointIter, Unicode, unicode_t )
	{
		const unicode_t CodePoint = CodePointIter.GetValue();
		if( CodePoint <= UTF8_1BYTERANGE )
		{
			const char Byte1 = static_cast<char>( CodePoint );
			UTF8Array.PushBack( Byte1 );
		}
		else if( CodePoint <= UTF8_2BYTERANGE )
		{
			const char Byte1 = UTF8_2BYTEMARKER | static_cast<char>( ( CodePoint >> UTF8_CBYTESHIFT ) );
			const char Byte2 = UTF8_CBYTEMARKER | static_cast<char>( CodePoint & UTF8_CBYTEMASKLOW );
			UTF8Array.PushBack( Byte1 );
			UTF8Array.PushBack( Byte2 );
		}
		else if( CodePoint <= UTF8_3BYTERANGE )
		{
			const char Byte1 = UTF8_3BYTEMARKER | static_cast<char>( CodePoint >> ( 2 * UTF8_CBYTESHIFT ) );
			const char Byte2 = UTF8_CBYTEMARKER | static_cast<char>( ( CodePoint >> UTF8_CBYTESHIFT ) & UTF8_CBYTEMASKLOW );
			const char Byte3 = UTF8_CBYTEMARKER | static_cast<char>( CodePoint & UTF8_CBYTEMASKLOW );
			UTF8Array.PushBack( Byte1 );
			UTF8Array.PushBack( Byte2 );
			UTF8Array.PushBack( Byte3 );
		}
		else
		{
			const char Byte1 = UTF8_4BYTEMARKER | static_cast<char>( CodePoint >> ( 3 * UTF8_CBYTESHIFT ) );
			const char Byte2 = UTF8_CBYTEMARKER | static_cast<char>( CodePoint >> ( 2 * UTF8_CBYTESHIFT ) & UTF8_CBYTEMASKLOW );
			const char Byte3 = UTF8_CBYTEMARKER | static_cast<char>( ( CodePoint >> UTF8_CBYTESHIFT ) & UTF8_CBYTEMASKLOW );
			const char Byte4 = UTF8_CBYTEMARKER | static_cast<char>( CodePoint & UTF8_CBYTEMASKLOW );
			UTF8Array.PushBack( Byte1 );
			UTF8Array.PushBack( Byte2 );
			UTF8Array.PushBack( Byte3 );
			UTF8Array.PushBack( Byte4 );
		}
	}

	UTF8Array.PushBack( '\0' );

	return SimpleString( UTF8Array );
}

// Parse a code point string (e.g., "U+0020")
unicode_t SimpleString::GetCodePoint() const
{
	unicode_t	CodePoint = 0;

	FOR_EACH_INDEX( CharIndex, m_Length )
	{
		const char c = m_String[ CharIndex ];
		if( c >= '0' && c <= '9' )
		{
			const unicode_t CharValue = static_cast<unicode_t>( c - '0' );
			CodePoint = ( CodePoint << 4 ) | CharValue;
		}
		else if( c >= 'a' && c <= 'f' )
		{
			const unicode_t CharValue = 10 + static_cast<unicode_t>( c - 'a' );
			CodePoint = ( CodePoint << 4 ) | CharValue;
		}
		else if( c >= 'A' && c <= 'F' )
		{
			const unicode_t CharValue = 10 + static_cast<unicode_t>( c - 'A' );
			CodePoint = ( CodePoint << 4 ) | CharValue;
		}
	}

	return CodePoint;
}

// Returns "U+xxxx" form
/*static*/ SimpleString SimpleString::GetCodePointString( const unicode_t CodePoint )
{
	ASSERT( CodePoint < 0x10000 );
	return PrintF( "U+%04X", CodePoint );
}