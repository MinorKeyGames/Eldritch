#ifndef FILE_H
#define FILE_H

#include "simplestring.h"
#include "array.h"

#if BUILD_WINDOWS
#include <Windows.h>
#endif

#if BUILD_WINDOWS
#define FOPEN( file, path, mode ) fopen_s( &file, path, mode )
#else
#define FOPEN( file, path, mode ) file = fopen( path, mode )
#endif

namespace FileUtil
{
	bool			Exists( const char* Filename );
	bool			PathExists( const char* Path );

	uint			Size( const char* Filename );
	bool			Compare( const char* Filename1, const char* Filename2 );
	SimpleString	Normalize( const char* Path );	// Makes all slashes forward and all characters lower-case

	void			MakePath( const char* Path );
	void			RecursiveMakePath( const char* Path );
	void			RemovePath( const char* Path );

	SimpleString	GetWorkingDirectory();
	void			ChangeWorkingDirectory( const char* Path );
	void			RecursiveRemoveDirectory( const char* Path );
	bool			RemoveFile( const char* Path );
	bool			Copy( const char* InPath, const char* OutPath, bool FailIfExists );
	bool			Move( const char* InPath, const char* OutPath );

	// Hacks used to clean up session folders; non-recursive (only returns folders in immediate path)
	void			GetAllFiles( const char* Path, Array< SimpleString >& OutFiles );
	void			GetAllFolders( const char* Path, Array< SimpleString >& OutFolders, bool AppendPath );

	const char*		StripLeadingUpFolders( const char* Path );	// Strips "//", "../", and "..\" (returns substring of Path, not new string)
	const char*		StripLeadingFolder( const char* Path, const char* Folder );		// Strips Folder if it leads Path (returns substring)
	const char*		StripLeadingFolders( const char* Path );	// Strips everything through the last "/" or "\" (returns substring of Path)
	SimpleString	StripExtension( const char* Path );			// Strips everything after and including the last '.' (returns new string)
	SimpleString	StripExtensions( const char* Path );		// Strips everything after and including the first '.' (returns new string)
	bool			SplitLeadingFolder( const char* Path, SimpleString& OutFolder, SimpleString& OutRemainder );

	// Windows folders
	SimpleString	GetUserPersonalPath();		// E.g., "My Documents", where user files go
	SimpleString	GetUserLocalAppDataPath();	// E.g., "AppData

	SimpleString	SanitizeFilename( const SimpleString& Filename );

	// Returns true if the process was launched.
	bool			Launch( const char* Filename );

	// Wrappers for common save/load dialog
#if BUILD_WINDOWS
	bool			GetSaveFile( const HWND& hWnd, const SimpleString& Desc, const SimpleString& Ext, SimpleString& OutFileName );
	bool			GetLoadFile( const HWND& hWnd, const SimpleString& Desc, const SimpleString& Ext, SimpleString& OutFileName );
#endif
}

#endif // FILE_H