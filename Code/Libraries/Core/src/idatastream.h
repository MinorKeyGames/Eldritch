#ifndef IDATASTREAM_H
#define IDATASTREAM_H

#include "simplestring.h"
#include "hashedstring.h"

// Abstract stream class
class IDataStream
{
public:
	virtual ~IDataStream() {}

	virtual int	Read( int NumBytes, void* Buffer ) const = 0;
	virtual int	Write( int NumBytes, const void* Buffer ) const = 0;
	virtual int PrintF( const char* Str, ... ) const = 0;
	virtual int	SetPos( int Position ) const = 0;
	virtual int	GetPos() const = 0;
	virtual int	EOS() const = 0;
	virtual int	Size() const = 0;

	// Helper functions
	inline void Skip( int NumBytes ) const { SetPos( GetPos() + NumBytes ); }

	//template< typename T > inline void WriteAuto( T t ) { Write( sizeof( T ), &t ); }
	inline void WriteUInt8( uint8 i ) const { Write( 1, &i ); }
	inline void WriteUInt16( uint16 i ) const { Write( 2, &i ); }
	inline void WriteUInt32( uint32 i ) const { Write( 4, &i ); }
	inline void WriteInt8( int8 i ) const { Write( 1, &i ); }
	inline void WriteInt16( int16 i ) const { Write( 2, &i ); }
	inline void WriteInt32( int32 i ) const { Write( 4, &i ); }
	inline void WriteFloat( float f ) const { Write( 4, &f ); }
	inline void WriteBool( bool b ) const { Write( 1, &b ); }
	inline void WriteHashedString( const HashedString& h ) const { Write( sizeof( HashedString ), &h ); }
	inline void WriteString( const SimpleString& String ) const
	{
		WriteUInt32( String.Length() + 1 );
		Write( String.Length() + 1, String.CStr() );
	}
	// Write string without leading length and trailing null, for
	// readable text. Probably best to prefer PrintF instead.
	inline void WritePlainString( const SimpleString& String ) const
	{
		Write( String.Length(), String.CStr() );
	}

	static inline uint SizeForWriteString( const SimpleString& String )
	{
		return 4 + String.Length() + 1;
	}

	//template< typename T > inline T ReadAuto() { T t; Read( sizeof( T ), &t ); return t; }
	inline uint8 ReadUInt8() const { uint8 i; Read( 1, &i ); return i; }
	inline uint16 ReadUInt16() const { uint16 i; Read( 2, &i ); return i; }
	inline uint32 ReadUInt32() const { uint32 i; Read( 4, &i ); return i; }
	inline int8 ReadInt8() const { int8 i; Read( 1, &i ); return i; }
	inline int16 ReadInt16() const { int16 i; Read( 2, &i ); return i; }
	inline int32 ReadInt32() const { int32 i; Read( 4, &i ); return i; }
	inline float ReadFloat() const { float f; Read( 4, &f ); return f; }
	inline bool ReadBool() const { bool b; Read( 1, &b ); return b; }
	inline HashedString ReadHashedString() const { HashedString h; Read( sizeof( HashedString ), &h ); return h; }
	inline SimpleString ReadString() const
	{
		uint Length = ReadUInt32();
		char* Buffer = new char[ Length ];
		Read( Length, Buffer );
		SimpleString RetVal = Buffer;
		delete Buffer;
		return RetVal;
	}
	inline SimpleString ReadCString() const
	{
		const int	StartPos	= GetPos();
		for( int8 Char = ReadInt8(); Char != '\0'; Char = ReadInt8() );
		const int	EndPos		= GetPos();
		const int	Length		= EndPos - StartPos;
		SetPos( StartPos );

		char* const	Buffer		= new char[ Length ];
		Read( Length, Buffer );

		SimpleString RetVal = Buffer;

		delete Buffer;
		return RetVal;
	}
};

#endif