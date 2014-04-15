#include "core.h"
#include "checksum.h"
#include "filestream.h"

int main( int argc, char* argv[] )
{
	if( argc > 1 )
	{
		bool SuppressedOutput = false;
		uint8* FileContents = NULL;
		int Length = 0;

		if( argc > 2 )
		{
			if( SimpleString( "-s" ) == argv[2] )
			{
				SuppressedOutput = true;
			}
		}

		{
			FileStream ChecksumFile( argv[1], FileStream::EFM_Read );
			Length = ChecksumFile.Size();
			FileContents = new uint8[ Length ];
			ChecksumFile.Read( Length, FileContents );
		}

		if( FileContents && Length )
		{
			uint32 FileChecksum = Checksum::Adler32( FileContents, Length );
			delete FileContents;

			if( SuppressedOutput )
			{
				PRINTF( "%s\n", argv[1] );
				PRINTF( "%d\n", Length );
				PRINTF( "0x%08X\n", FileChecksum );
			}
			else
			{
				PRINTF( "Filename: %s\n", argv[1] );
				PRINTF( "Length:   %d\n", Length );
				PRINTF( "Checksum: 0x%08X\n", FileChecksum );
			}
		}
	}

	return 0;
}