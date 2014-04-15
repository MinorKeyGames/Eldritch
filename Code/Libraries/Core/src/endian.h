#ifndef ENDIAN_H
#define ENDIAN_H

namespace Endian
{
	void SwapInPlace( uint32& i ) { i = ( ( i & 0xff000000 ) >> 24 ) | ( ( i & 0x00ff0000 ) >> 8 ) | ( ( i & 0x0000ff00 ) << 8 ) | ( ( i & 0x000000ff ) << 24 ); }
	void SwapInPlace( uint16& i ) { i = ( ( i & 0xff00 ) >> 8 ) | ( ( i & 0x00ff ) << 8 ); }

	uint32 Swap( const uint32 i ) { return ( ( i & 0xff000000 ) >> 24 ) | ( ( i & 0x00ff0000 ) >> 8 ) | ( ( i & 0x0000ff00 ) << 8 ) | ( ( i & 0x000000ff ) << 24 ); }
	uint16 Swap( const uint16 i ) { return ( ( i & 0xff00 ) >> 8 ) | ( ( i & 0x00ff ) << 8 ); }
}

#endif // ENDIAN_H