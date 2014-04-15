#include "core.h"
#include "configparser.h"
#include "configmanager.h"
#include "filestream.h"
#include "printmanager.h"
#include <string.h>
#include <stdio.h>

// Extensions:
// .cfg: Plaintext config used at runtime
// .config: Plaintext config compiled offline into .ccf
// .ccf: Compiled config baked from .config

// Syntax: ConfigCompiler.exe inconfig.config outconfig.ccf

int main( int argc, char* argv[] )
{
	SETPRINTLEVEL( PRINTLEVEL_Spam );

	if( argc != 3 )
	{
		printf( "Syntax: ConfigCompiler.exe <infile> <outfile>\n" );
		return 0;
	}

	ConfigParser::Parse( FileStream( argv[1], FileStream::EFM_Read ) );

	ConfigManager::Report();

	ConfigManager::Save( FileStream( argv[2], FileStream::EFM_Write ) );

	return 0;
}