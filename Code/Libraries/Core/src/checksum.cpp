#include "core.h"
#include "checksum.h"
#include "idatastream.h"

static uint32 Adler32Mod = 65521;

// Simple, inefficient implementation
uint32 Checksum::Adler32( uint8* Stream, uint Length )
{
	uint32 A = 1;
	uint32 B = 0;

	for( uint Index = 0; Index < Length; ++Index )
	{
		A = ( A + Stream[ Index ] ) % Adler32Mod;
		B = ( B + A ) % Adler32Mod;
	}

	return ( B << 16 ) | A;
}

uint32 Checksum::Adler32( const IDataStream& Stream )
{
	uint32 A = 1;
	uint32 B = 0;

	int Length = Stream.Size();
	for( int Index = 0; Index < Length && !Stream.EOS(); ++Index )
	{
		A = ( A + Stream.ReadUInt8() ) % Adler32Mod;
		B = ( B + A ) % Adler32Mod;
	}

	return ( B << 16 ) | A;
}