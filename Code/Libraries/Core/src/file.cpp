#include "core.h"
#include "file.h"

#include <stdio.h>
#include <string.h>

#if BUILD_WINDOWS
#include <direct.h>	// For directory functions
#include <shlobj.h>
#endif

#if BUILD_LINUX || BUILD_MAC
#include <unistd.h>
#include <sys/stat.h>
#endif

// This unit is a mess, a mix of standard and Windows-only functions, many with special-case
// purposes and some that are unsafe to use in-game and are only ever meant to run in tools.

// Cross-platform verified.
// Match upper and lower case letters and slashes
bool Match( char c1, char c2 )
{
	if( c1 == c2 )
	{
		return true;
	}
	if( ( c1 == '\\' && c2 == '/' ) ||
		( c1 == '/' && c2 == '\\' ) )
	{
		return true;
	}
	if( ( c1 >= 'A' && c1 <= 'Z' ) && ( c1 == ( c2 - 32 ) ) )
	{
		return true;
	}
	if( ( c1 >= 'a' && c1 <= 'z' ) && ( c1 == ( c2 + 32 ) ) )
	{
		return true;
	}
	return false;
}

// Cross-platform verified.
bool FileUtil::Exists( const char* Filename )
{
	FILE* pFile = NULL;
	FOPEN( pFile, Filename, "rb" );
	if( pFile )
	{
		fclose( pFile );
		return true;
	}
	return false;
}

// Cross-platform verified.
bool FileUtil::PathExists( const char* Path )
{
	SimpleString CurrentDir = GetWorkingDirectory();
#if BUILD_WINDOWS
	if( _chdir( Path ) )
#else
	if( chdir( Path ) )
#endif
	{
		return false;
	}
	else
	{
#if BUILD_WINDOWS
		_chdir( CurrentDir.CStr() );
#else
		chdir( CurrentDir.CStr() );
#endif
		return true;
	}
}

// Cross-platform verified.
uint FileUtil::Size( const char* Filename )
{
	FILE* pFile = NULL;
	FOPEN( pFile, Filename, "rb" );
	if( pFile )
	{
		fseek( pFile, 0, SEEK_END );
		uint RetVal = ftell( pFile );
		fclose( pFile );
		return RetVal;
	}
	return 0;
}

// Cross-platform verified.
// Compare filenames, case-insensitive, / and \ equivalent
bool FileUtil::Compare( const char* Filename1, const char* Filename2 )
{
	const char* Iter1 = Filename1;
	const char* Iter2 = Filename2;
	ASSERT( Iter1 );
	ASSERT( Iter2 );
	while( *Iter1 && *Iter2 )
	{
		if( !Match( *Iter1, *Iter2 ) )
		{
			return false;
		}
		++Iter1;
		++Iter2;
	}
	return true;
}

// Cross-platform verified.
SimpleString FileUtil::Normalize( const char* Path )
{
	ASSERT( Path );

	uint Length = (uint)strlen( Path ) + 1;
	char* NormalizedPath = new char[ Length ];

	strcpy_s( NormalizedPath, Length, Path );

	for( char* c = NormalizedPath; *c; ++c )
	{
		if( *c == '\\' )
		{
			*c = '/';
		}
		else if( *c >= 'A' && *c <= 'Z' )
		{
			*c += 32;
		}
	}

	SimpleString RetVal( NormalizedPath );	// Causes an extra allocation, which bugs me, but I don't want to make SimpleString's char* accessible
	delete NormalizedPath;
	return RetVal;
}

// Cross-platform verified.
void FileUtil::MakePath( const char* Path )
{
#if BUILD_WINDOWS
	_mkdir( Path );
#else
	mkdir( Path, 0x0777 );
#endif
}

// Cross-platform verified.
void FileUtil::RecursiveMakePath( const char* Path )
{
	SimpleString Filepath = Path;
	SimpleString SplitPath;
	SimpleString Remainder;

	int DescendDepth = 0;
	while( FileUtil::SplitLeadingFolder( Filepath.CStr(), SplitPath, Remainder ) )
	{
		if( !FileUtil::PathExists( SplitPath.CStr() ) )
		{
			FileUtil::MakePath( SplitPath.CStr() );
		}
		FileUtil::ChangeWorkingDirectory( SplitPath.CStr() );
		Filepath = Remainder;
		DescendDepth++;
	}

	while( DescendDepth > 0 )
	{
		FileUtil::ChangeWorkingDirectory( ".." );
		--DescendDepth;
	}
}

// Cross-platform verified.
void FileUtil::RemovePath( const char* Path )
{
#if BUILD_WINDOWS
	_rmdir( Path );
#else
	rmdir( Path );
#endif
}

// Cross-platform verified.
SimpleString FileUtil::GetWorkingDirectory()
{
	char WorkingDir[ 256 ];
#if BUILD_WINDOWS
	_getcwd( WorkingDir, 256 );
#else
	getcwd( WorkingDir, 256 );
#endif
	return SimpleString( WorkingDir );
}

// Cross-platform verified.
void FileUtil::ChangeWorkingDirectory( const char* Path )
{
	ASSERT( Path );
#if BUILD_WINDOWS
	_chdir( Path );
#else
	chdir( Path );
#endif
}

// Mixing in some Windows calls with all the POSIX stuff in the
// rest of this. Dur. Based on http://www.codeguru.com/forum/showthread.php?t=239271
void FileUtil::RecursiveRemoveDirectory( const char* Path )
{
#if BUILD_WINDOWS
	HANDLE          FileHandle;
	SimpleString	FilePath;
	SimpleString	FilePattern = SimpleString( Path ) + "\\*.*";
	WIN32_FIND_DATA FileInformation;

	FileHandle = FindFirstFile( FilePattern.CStr(), &FileInformation );
	if( FileHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( FileInformation.cFileName[0] != '.' )	// Don't recurse up tree
			{
				FilePath = SimpleString( Path ) + "\\" + FileInformation.cFileName;

				if( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					// Delete subdirectory
					RecursiveRemoveDirectory( FilePath.CStr() );
				}
				else
				{
					// Set file attributes so we can delete
					if( SetFileAttributes( FilePath.CStr(), FILE_ATTRIBUTE_NORMAL ) )
					{
						DeleteFile( FilePath.CStr() );
					}
				}
			}
		}
		while( TRUE == FindNextFile( FileHandle, &FileInformation ) );

		FindClose( FileHandle );

		// Set directory attributes so we can delete
		if( SetFileAttributes( Path, FILE_ATTRIBUTE_NORMAL ) )
		{
			RemoveDirectory( Path );
		}
	}
#else
	// TODO PORT LATER (not currently used)
	Unused( Path );
	WARN;
#endif
}

// Cross-platform verified.
bool FileUtil::RemoveFile( const char* Path )
{
#if BUILD_WINDOWS
	return FALSE != DeleteFile( Path );
#else
	return 0 == unlink( Path );
#endif
}

bool FileUtil::Copy( const char* InPath, const char* OutPath, bool FailIfExists )
{
#if BUILD_WINDOWS
	return FALSE != CopyFile( InPath, OutPath, FailIfExists );
#else
	// TODO PORT LATER (not currently used)
	Unused( InPath );
	Unused( OutPath );
	Unused( FailIfExists );
	WARN;
	return false;
#endif
}

bool FileUtil::Move( const char* InPath, const char* OutPath )
{
#if BUILD_WINDOWS
	return FALSE != MoveFile( InPath, OutPath );
#else
	// TODO PORT LATER (not currently used)
	Unused( InPath );
	Unused( OutPath );
	WARN;
	return false;
#endif
}

// Hacks used to clean up session folders; non-recursive (only returns folders in immediate path)
void FileUtil::GetAllFiles( const char* Path, Array< SimpleString >& OutFiles )
{
#if BUILD_WINDOWS
	HANDLE          FileHandle;
	SimpleString	FilePath;
	SimpleString	FilePattern = SimpleString( Path ) + "\\*.*";
	WIN32_FIND_DATA FileInformation;

	FileHandle = FindFirstFile( FilePattern.CStr(), &FileInformation );
	if( FileHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( FileInformation.cFileName[0] != '.' )	// Don't recurse up tree
			{
				FilePath = SimpleString( Path ) + "\\" + FileInformation.cFileName;

				if( 0 == ( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
				{
					OutFiles.PushBack( FilePath.CStr() );
				}
			}
		}
		while( TRUE == FindNextFile( FileHandle, &FileInformation ) );

		FindClose( FileHandle );
	}
#else
	// TODO PORT LATER (not currently used)
	Unused( Path );
	Unused( OutFiles );
	WARN;
#endif
}

void FileUtil::GetAllFolders( const char* Path, Array< SimpleString >& OutFolders, bool AppendPath )
{
#if BUILD_WINDOWS
	HANDLE          FileHandle;
	SimpleString	FilePath;
	SimpleString	FilePattern = SimpleString( Path ) + "\\*.*";
	WIN32_FIND_DATA FileInformation;

	FileHandle = FindFirstFile( FilePattern.CStr(), &FileInformation );
	if( FileHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			if( FileInformation.cFileName[0] != '.' )	// Don't recurse up tree
			{
				if( AppendPath )
				{
					FilePath = SimpleString( Path ) + "\\" + FileInformation.cFileName;
				}
				else
				{
					FilePath = FileInformation.cFileName;
				}

				if( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					OutFolders.PushBack( FilePath.CStr() );
				}
			}
		}
		while( TRUE == FindNextFile( FileHandle, &FileInformation ) );

		FindClose( FileHandle );
	}
#else
	// TODO PORT LATER (not currently used)
	Unused( Path );
	Unused( OutFolders );
	Unused( AppendPath );
	WARN;
#endif
}

// Cross-platform verified.
const char* FileUtil::StripLeadingUpFolders( const char* Path )
{
	ASSERT( Path );
	while( *Path )
	{
		if( Match( *Path, '/' ) && Match( *( Path + 1 ), '/' ) )
		{
			Path += 2;
		}
		else if( Match( *Path, '.' ) && Match( *( Path + 1 ), '.' ) && Match( *( Path + 2 ), '/' ) )
		{
			Path += 3;
		}
		else
		{
			return Path;
		}
	}
	return Path;
}

// Cross-platform verified.
const char* FileUtil::StripLeadingFolder( const char* Path, const char* Folder )
{
	ASSERT( Path );
	ASSERT( Folder );
	const char* OriginalPath = Path;
	while( *Path && *Folder )
	{
		if( Match( *Path, *Folder ) )
		{
			++Path;
			++Folder;
		}
		else
		{
			return OriginalPath;	// Folder wasn't matched, don't modify
		}
	}
	if( Match( *Path, '/' ) )
	{
		return Path + 1;
	}
	else
	{
		return OriginalPath;
	}
}

// Cross-platform verified.
const char* FileUtil::StripLeadingFolders( const char* Path )
{
	ASSERT( Path );

	const char* LastPathChar = NULL;
	while( *Path )
	{
		if( Match( *Path, '/' ) )
		{
			LastPathChar = Path;
		}
		++Path;
	}

	// LastPathChar is now at the position of the last path character
	if( LastPathChar )
	{
		return LastPathChar + 1;
	}
	else
	{
		return Path;	// No path characters, just return the original string
	}
}

// Cross-platform verified.
SimpleString FileUtil::StripExtension( const char* Path )
{
	ASSERT( Path );

	uint Length = (uint)strlen( Path ) + 1;
	char* PathCopy = new char[ Length ];
	strcpy_s( PathCopy, Length, Path );

	char* PathCopyIter = PathCopy;
	char* LastDot = NULL;
	while( *PathCopyIter )
	{
		if( Match( *PathCopyIter, '.' ) )
		{
			LastDot = PathCopyIter;
		}
		++PathCopyIter;
	}

	// LastDot is now at the position of the last dot
	if( LastDot )
	{
		*LastDot = '\0';
	}

	SimpleString RetVal( PathCopy );
	delete PathCopy;
	return RetVal;
}

// Cross-platform verified.
SimpleString FileUtil::StripExtensions( const char* Path )
{
	ASSERT( Path );

	uint Length = (uint)strlen( Path ) + 1;
	char* PathCopy = new char[ Length ];
	strcpy_s( PathCopy, Length, Path );

	char* PathCopyIter = PathCopy;
	char* FirstDot = NULL;
	while( *PathCopyIter && !FirstDot )
	{
		if( Match( *PathCopyIter, '.' ) )
		{
			FirstDot = PathCopyIter;
		}
		++PathCopyIter;
	}

	// FirstDot is now at the position of the first dot
	if( FirstDot )
	{
		*FirstDot = '\0';
	}

	SimpleString RetVal( PathCopy );
	delete PathCopy;
	return RetVal;
}

// Cross-platform verified.
// This probably isn't super robust, but will work fine for a basic path.
bool FileUtil::SplitLeadingFolder( const char* Path, SimpleString& OutFolder, SimpleString& OutRemainder )
{
	ASSERT( Path );

	SimpleString PathCopy( Path );
	char* FolderPath = PathCopy.MutableCStr();
	char* IterStr = PathCopy.MutableCStr();

	while( *IterStr )
	{
		if( Match( *IterStr, '/' ) )
		{
			// Split here
			*IterStr = '\0';
			OutFolder = FolderPath;
			OutRemainder = IterStr + 1;
			return true;
		}
		++IterStr;
	}

	return false;
}

SimpleString FileUtil::GetUserPersonalPath()
{
#if BUILD_WINDOWS
	char Path[ MAX_PATH ];
	if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, Path ) ) )
	{
		return SimpleString( Path );
	}
	else
	{
		return SimpleString( "./" );
	}
#else
	// TODO PORT LATER (not currently used)
	WARN;
	return SimpleString( "./" );
#endif
}

SimpleString FileUtil::GetUserLocalAppDataPath()
{
#if BUILD_WINDOWS
	char Path[ MAX_PATH ];
	if( SUCCEEDED( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, Path ) ) )
	{
		return SimpleString( Path );
	}
	else
	{
		return SimpleString( "./" );
	}
#else
	// TODO PORT LATER (not currently used)
	WARN;
	return SimpleString( "./" );
#endif
}

// Cross-platform verified.
char CharToChar( char Char )	// PROBLEM, CODE REVIEWER? :D
{
	Char = Char & 0xf;
	if( Char < 10 )
	{
		return '0' + Char;
	}
	else
	{
		return 'A' + ( Char - 10 );
	}
}

// Cross-platform verified.
SimpleString FileUtil::SanitizeFilename( const SimpleString& Filename )
{
	Array<char> SanitizedFilename;
	Filename.FillArray( SanitizedFilename, true );
	for( uint CharIndex = 0; CharIndex < SanitizedFilename.Size(); ++CharIndex )
	{
		if( SanitizedFilename[ CharIndex ] == '\\' ||
			SanitizedFilename[ CharIndex ] == '/' ||
			SanitizedFilename[ CharIndex ] == ':' ||
			SanitizedFilename[ CharIndex ] == '*' ||
			SanitizedFilename[ CharIndex ] == '?' ||
			SanitizedFilename[ CharIndex ] == '"' ||
			SanitizedFilename[ CharIndex ] == '<' ||
			SanitizedFilename[ CharIndex ] == '>' ||
			SanitizedFilename[ CharIndex ] == '|' )
		{
			char Char = SanitizedFilename[ CharIndex ];

			SanitizedFilename.Remove( CharIndex );
			SanitizedFilename.Insert( CharToChar( Char & 0xf ), CharIndex );
			SanitizedFilename.Insert( CharToChar( Char >> 4 ), CharIndex );
			SanitizedFilename.Insert( '%', CharIndex );
		}
	}

	return SimpleString( SanitizedFilename );
}

bool TryShellExecute( const char* Filename, const char* Operation )
{
#if BUILD_WINDOWS
	HINSTANCE Result = ShellExecute(
		NULL,			// hWnd
		Operation,		// Operation (runas requests elevated access)
		Filename,		// File
		NULL,			// Parameters (or arguments to program)
		NULL,			// Directory
		SW_SHOWNORMAL	// Show flags (this makes sense in the context of opening a file in some editor)
		);

	const INT ExecuteResult = PtrToInt( Result );
	if( ExecuteResult > 32 )
	{
		return true;
	}
	else
	{
		PRINTF( "ShellExecute failed with code %d\n", ExecuteResult );
		return false;
	}
#else
	// TODO PORT LATER (not currently used)
	Unused( Filename );
	Unused( Operation );
	WARN;
	return false;
#endif
}

bool FileUtil::Launch( const char* Filename )
{
#if BUILD_WINDOWS
	if( TryShellExecute( Filename, "runas" ) )
	{
		return true;
	}

	if( TryShellExecute( Filename, "open" ) )
	{
		return true;
	}

	return false;
#else
	// TODO PORT LATER (not currently used)
	Unused( Filename );
	WARN;
	return false;
#endif
}

#if BUILD_WINDOWS
// TODO PORT LATER (not currently used)
bool FileUtil::GetSaveFile( const HWND& hWnd, const SimpleString& Desc, const SimpleString& Ext, SimpleString& OutFileName )
{
	// HACK, because string functions don't like dealing with \0 as part of a string.
	const SimpleString Filter = SimpleString::PrintF( "%s (*.%s)0*.%s0All Files (*.*)0*.*0", Desc.CStr(), Ext.CStr(), Ext.CStr() );
	Filter.Replace( '0', '\0' );

	OPENFILENAME OpenFileName;
	const uint MaxFilenameSize = 256;
	char FilenameBuffer[ MaxFilenameSize ];
	memset( &OpenFileName, 0, sizeof( OpenFileName ) );
	FilenameBuffer[0] = '\0';
	OpenFileName.lStructSize = sizeof( OpenFileName );
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = Filter.CStr();
	OpenFileName.nFilterIndex = 1;
	OpenFileName.lpstrFile = FilenameBuffer;
	OpenFileName.nMaxFile = MaxFilenameSize;
	OpenFileName.lpstrInitialDir = ".";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;
	OpenFileName.lpstrDefExt = Ext.CStr();

	const SimpleString WorkingDirectory = FileUtil::GetWorkingDirectory();
	const BOOL Success = GetSaveFileName( &OpenFileName );
	FileUtil::ChangeWorkingDirectory( WorkingDirectory.CStr() );

	if( Success )
	{
		OutFileName = OpenFileName.lpstrFile;
		return true;
	}
	else
	{
		const DWORD Error = CommDlgExtendedError();
		PRINTF( "Error getting save file name: %d\n", Error );	// See cderr.h for meaning.
		return false;
	}
}
#endif

#if BUILD_WINDOWS
// TODO PORT LATER (not currently used)
bool FileUtil::GetLoadFile( const HWND& hWnd, const SimpleString& Desc, const SimpleString& Ext, SimpleString& OutFileName )
{
	// HACK, because string functions don't like dealing with \0 as part of a string.
	const SimpleString Filter = SimpleString::PrintF( "%s (*.%s)0*.%s0All Files (*.*)0*.*0", Desc.CStr(), Ext.CStr(), Ext.CStr() );
	Filter.Replace( '0', '\0' );

	OPENFILENAME OpenFileName;
	const uint MaxFilenameSize = 256;
	char FilenameBuffer[ MaxFilenameSize ];
	memset( &OpenFileName, 0, sizeof( OpenFileName ) );
	FilenameBuffer[0] = '\0';
	OpenFileName.lStructSize = sizeof( OpenFileName );
	OpenFileName.hwndOwner = hWnd;
	OpenFileName.lpstrFilter = Filter.CStr();
	OpenFileName.nFilterIndex = 1;
	OpenFileName.lpstrFile = FilenameBuffer;
	OpenFileName.nMaxFile = MaxFilenameSize;
	OpenFileName.lpstrInitialDir = ".";
	OpenFileName.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
	OpenFileName.lpstrDefExt = Ext.CStr();

	const SimpleString WorkingDirectory = FileUtil::GetWorkingDirectory();
	const BOOL Success = GetOpenFileName( &OpenFileName );
	FileUtil::ChangeWorkingDirectory( WorkingDirectory.CStr() );

	if( Success )
	{
		OutFileName = OpenFileName.lpstrFile;
		return true;
	}
	else
	{
		const DWORD Error = CommDlgExtendedError();
		PRINTF( "Error getting load file name: %d\n", Error );	// See cderr.h for meaning.
		return false;
	}
}
#endif