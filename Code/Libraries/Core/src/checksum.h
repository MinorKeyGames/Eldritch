#ifndef CHECKSUM_H
#define CHECKSUM_H

class IDataStream;

namespace Checksum
{
	uint32	Adler32( uint8* Stream, uint Length );
	uint32	Adler32( const IDataStream& Stream );
}

#endif // CHECKSUM_H