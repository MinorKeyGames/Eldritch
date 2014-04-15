#include "core.h"
#include "filepacker.h"
#include "printmanager.h"

#include <string.h>

int main( int argc, char* argv[] )
{
	if( argc < 3 || argc > 4 )
	{
		PRINTF( "Syntax:\n\tFilePacker.exe <infile> <packfile> [-c]\n" );
		PRINTF( "\tFilePacker.exe -u <packfile>\n" );
		return 0;
	}

	FilePacker Packer;
	if( 0 == strcmp( "-u", argv[1] ) )
	{
		return Packer.UnpackFile( argv[2] );
	}
	else
	{
		bool Compress = false;
		if( argc == 4 && 0 == strcmp( "-c", argv[3] ) )
		{
			Compress = true;
		}
		return Packer.PackFile( argv[1], argv[2], Compress );
	}
}