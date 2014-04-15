#include "core.h"
#include "memorystream.h"

#include <memory.h>
#include <stdarg.h>
#include <stdio.h>

MemoryStream::MemoryStream()
:	m_Data( NULL )
,	m_Size( 0 )
,	m_Offset( 0 ) {}

MemoryStream::MemoryStream( void* pData, uint Size )
:	m_Data( ( byte* )pData )
,	m_Size( Size )
,	m_Offset( 0 ) {}

int MemoryStream::Read( int NumBytes, void* Buffer ) const
{
	ASSERT( m_Size == 0 || m_Offset + NumBytes <= m_Size );
	if( m_Offset + NumBytes <= m_Size )
	{
		memcpy_s( Buffer, NumBytes, m_Data + m_Offset, NumBytes );
		m_Offset += NumBytes;
		return 1;
	}
	return 0;
}

int	MemoryStream::Write( int NumBytes, const void* Buffer ) const
{
	ASSERT( m_Size == 0 || m_Offset + NumBytes <= m_Size );
	if( m_Offset + NumBytes <= m_Size )
	{
		memcpy_s( m_Data + m_Offset, NumBytes, Buffer, NumBytes );
		m_Offset += NumBytes;
		return 1;
	}
	return 0;
}

int MemoryStream::PrintF( const char* Str, ... ) const
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

int	MemoryStream::SetPos( int Position ) const
{
	m_Offset = Position;
	return 1;
}

int	MemoryStream::GetPos() const
{
	return (int)m_Offset;
}

int MemoryStream::EOS() const
{
	if( m_Size && m_Offset >= m_Size )
	{
		return 1;
	}
	return 0;
}

int MemoryStream::Size() const
{
	return (int)m_Size;
}

const byte* MemoryStream::GetReadPointer() const
{
	return m_Data + m_Offset;
}

int MemoryStream::GetRemainingSize() const
{
	return m_Size - m_Offset;
}