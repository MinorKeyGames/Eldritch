#include "core.h"
#include "filestream.h"

#include <stdarg.h>

FileStream::FileStream()
:	m_TheFile( NULL )
,	m_FileMode( EFM_None )
{
}

FileStream::FileStream( const char* FileName, EFileMode FileMode )
:	m_TheFile( NULL )
,	m_FileMode( FileMode )
{
	if( FileMode == EFM_Read )
	{
		FOPEN( m_TheFile, FileName, "rb" );
	}
	else if( FileMode == EFM_Write )
	{
		FOPEN( m_TheFile, FileName, "wb" );
	}
	else if( FileMode == EFM_Append )
	{
		FOPEN( m_TheFile, FileName, "ab" );
	}
#if BUILD_DEV
	if( !m_TheFile )
	{
		PrintManager* const pPrintManager = PrintManager::GetInstance_NoAlloc();
		if( pPrintManager && FileName == pPrintManager->GetLogFilename() )
		{
			// Don't print and don't assert! That would cause endless recursion trying to write to a log file that can't be opened.
			return;
		}
		
		PRINTF( "Couldn't open file: %s\n", FileName );
		WARN;
	}
#endif
}

FileStream::~FileStream()
{
	fclose( m_TheFile );
}

int FileStream::Read( int NumBytes, void* Buffer ) const
{
	ASSERT( m_FileMode == EFM_Read );

	return ( int )fread( Buffer, NumBytes, 1, m_TheFile );
}

int FileStream::Write( int NumBytes, const void* Buffer ) const
{
	ASSERT( m_FileMode == EFM_Write || m_FileMode == EFM_Append );

	return ( int )fwrite( Buffer, NumBytes, 1, m_TheFile );
}

int FileStream::PrintF( const char* Str, ... ) const
{
	ASSERT( m_FileMode == EFM_Write || m_FileMode == EFM_Append );

	va_list	Args;
	int		Length	= 0;
	char*	Buffer	= NULL;
	int		RetVal	= 0;

	va_start( Args, Str );
	Length = VSPRINTF_COUNT( Str, Args ) + 1;
	Buffer = new char[ Length ];	// TODO: Pool this instead of dynamically allocating
	VSPRINTF( Buffer, Length, Str, Args );

	RetVal = Write( Length - 1, Buffer );

	SafeDelete( Buffer );
	return RetVal;
}

int FileStream::SetPos( int Position ) const
{
	return ( int )fseek( m_TheFile, Position, SEEK_SET );
}

int FileStream::GetPos() const
{
	return ftell( m_TheFile );
}

int FileStream::EOS() const
{
	return feof( m_TheFile );
}

int FileStream::Size() const
{
	int Pos = GetPos();
	fseek( m_TheFile, 0, SEEK_END );
	int RetVal = GetPos();
	SetPos( Pos );
	return RetVal;
}