#include "core.h"
#include "bufferedfilestream.h"

#include <stdio.h>
#include <stdarg.h>
#include <memory.h>

BufferedFileStream::BufferedFileStream()
:	m_Filename( NULL )
,	m_Buffer()
,	m_FileMode( EFM_None )
,	m_Position( 0 )
,	m_Filesize( 0 )
{
}

BufferedFileStream::BufferedFileStream( const char* Filename, EFileMode FileMode )
:	m_Filename( Filename )
,	m_Buffer()
,	m_FileMode( FileMode )
,	m_Position( 0 )
,	m_Filesize( 0 )
{
	if( m_FileMode == EFM_Read )
	{
		FILE* pFile = NULL;
		FOPEN( pFile, m_Filename, "rb" );
		if( pFile )
		{
			fseek( pFile, 0, SEEK_END );
			m_Filesize = ftell( pFile );
			fseek( pFile, 0, SEEK_SET );
			if( m_Filesize )
			{
				m_Buffer.Resize( m_Filesize );
				fread( m_Buffer.GetData(), 1, m_Filesize, pFile );
			}
			fclose( pFile );
		}
	}
}

BufferedFileStream::~BufferedFileStream()
{
	if( m_FileMode == EFM_Write || m_FileMode == EFM_Append )
	{
		if( m_Filesize )
		{
			FILE* pFile = NULL;
			if( m_FileMode == EFM_Write )
			{
				FOPEN( pFile, m_Filename, "wb" );
			}
			else
			{
				FOPEN( pFile, m_Filename, "ab" );
			}
			fwrite( m_Buffer.GetData(), m_Filesize, 1, pFile );
			fclose( pFile );
		}
	}
	m_Buffer.Clear();
}

int BufferedFileStream::Read( int NumBytes, void* Buffer ) const
{
	if( NumBytes > 0 )
	{
		memcpy( Buffer, &m_Buffer[ m_Position ], NumBytes );
		m_Position += NumBytes;
	}
	return 1;
}

int BufferedFileStream::Write( int NumBytes, const void* Buffer ) const
{
	if( NumBytes > 0 )
	{
		m_Buffer.Resize( m_Filesize + NumBytes );
		memcpy( &m_Buffer[ m_Position ], Buffer, NumBytes );
		m_Filesize += NumBytes;
		m_Position += NumBytes;
	}
	return 1;
}

int BufferedFileStream::PrintF( const char* Str, ... ) const
{
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

int BufferedFileStream::SetPos( int Position ) const
{
	m_Position = Position;
	return 1;
}

int BufferedFileStream::GetPos() const
{
	return m_Position;
}

int BufferedFileStream::EOS() const
{
	ASSERT( m_FileMode == EFM_Read );
	return m_Position >= m_Filesize;
}

int BufferedFileStream::Size() const
{
	return m_Filesize;
}