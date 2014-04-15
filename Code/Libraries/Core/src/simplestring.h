#ifndef SIMPLESTRING_H
#define SIMPLESTRING_H

#include "array.h"

typedef uint32	unicode_t;	// Represents a single Unicode code point in 4 bytes

class SimpleString
{
public:
	explicit SimpleString( bool NoInit );		// To prevent static scope strings from being initialized with the wrong allocator
	SimpleString( const char* pString = "" );
	SimpleString( const char* pNonTerminatedString, uint Length );
	SimpleString( const SimpleString& String );
	SimpleString( const Array<char>& String );	// Be careful, the array must be null-terminated
	~SimpleString();

	SimpleString&	operator=( const SimpleString& String );
	SimpleString&	operator=( const char* pString );
	SimpleString&	operator=( const Array< char >& String );	// Be careful that the array might not be null-terminated
	SimpleString	operator+( const SimpleString& String ) const;
	SimpleString&	operator+=( const SimpleString& String );

	bool			operator==( const SimpleString& String ) const;
	bool			operator!=( const SimpleString& String ) const;
	bool			operator<( const SimpleString& String ) const;
	bool			StrICmp( const SimpleString& String ) const;

	const char*		CStr() const;
	char*			MutableCStr();
	uint			Length() const; // Doesn't include the terminating null
	char			GetChar( uint Index ) const { ASSERT( Index < m_Length ); return m_String[ Index ]; }

	bool			Contains( const SimpleString& String ) const;
	bool			BeginsWith( const SimpleString& String ) const;

	void			Replace( const char Char, const char ReplaceChar ) const;
	SimpleString	Replace( const char* const Find, const char* const Replace ) const;
	void			SplitFind( const char Delimiter, SimpleString& Left, SimpleString& Right ) const;	// Split at the first instance of Delimiter
	void			Split( uint Index, SimpleString& Left, SimpleString& Right ) const;
	uint			Find( const char* Find ) const;

	bool			AsBool() const;
	int				AsInt() const;
	float			AsFloat() const;

	void			FillArray( Array<char>& OutArray, bool WithNull = false ) const;	// Clears and fills given array, optionally including terminating null

	void				UTF8ToUnicode( Array<unicode_t>& OutUnicode ) const;				// Converts string (assumed in UTF-8 form without BOM) to an array of Unicode code points
	static SimpleString	SetUTF8( const Array<unicode_t>& Unicode );							// Converts array of Unicode code points to UTF-8 form
	unicode_t			GetCodePoint() const;												// Converts string (assumed in "U+xxxx" form) to a Unicode code point
	static SimpleString	GetCodePointString( const unicode_t CodePoint );					// Returns "U+xxxx" form

	SimpleString		EscapeSequenceEncode() const;
	SimpleString		URLEncode() const;
	SimpleString		URLEncodeUTF8() const;

	SimpleString		ToLower() const;

	static SimpleString	PrintF( const char* FormatString, ... );

	static void			InitializeAllocator( uint Size );
	static void			ShutDownAllocator();
	static void			ReportAllocator( const SimpleString& Filename );

private:
	void				Initialize( const char* pString );
	void				InitializeNonTerminated( const char* pString, uint Length );
	static char*		Allocate( uint Size );
	static Allocator&	GetAllocator( uint Size );

	char*	m_String;
	uint	m_Length;	// Doesn't include the terminating null!

	static Allocator	m_AllocatorSmall;
	static Allocator	m_AllocatorLarge;
	static bool			m_UsingAllocator;
};

#endif // SIMPLESTRING_H