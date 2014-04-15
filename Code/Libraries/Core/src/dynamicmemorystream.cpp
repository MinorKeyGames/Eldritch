#include "core.h"
#include "dynamicmemorystream.h"

#include <stdarg.h>
#include <stdio.h>

DynamicMemoryStream::DynamicMemoryStream()
:	m_ByteArray()
,	m_Offset( 0 )
{
}

int DynamicMemoryStream::Read( int NumBytes, void* Buffer ) const
{
	ASSERT( m_ByteArray.Size() == 0 || m_Offset + NumBytes <= m_ByteArray.Size() );
	if( m_Offset + NumBytes <= m_ByteArray.Size() )
	{
		memcpy_s( Buffer, NumBytes, m_ByteArray.GetData() + m_Offset, NumBytes );
		m_Offset += NumBytes;
		return 1;
	}
	return 0;
}

int	DynamicMemoryStream::Write( int NumBytes, const void* Buffer ) const
{
	if( m_Offset + NumBytes > m_ByteArray.Size() )
	{
		m_ByteArray.Resize( m_Offset + NumBytes );
	}

	memcpy_s( m_ByteArray.GetData() + m_Offset, NumBytes, Buffer, NumBytes );
	m_Offset += NumBytes;

	return 1;
}

int DynamicMemoryStream::PrintF( const char* Str, ... ) const
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

int	DynamicMemoryStream::SetPos( int Position ) const
{
	m_Offset = Position;
	return 1;
}

int	DynamicMemoryStream::GetPos() const
{
	return static_cast<int>( m_Offset );
}

int DynamicMemoryStream::EOS() const
{
	if( m_ByteArray.Size() && m_Offset >= m_ByteArray.Size() )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int DynamicMemoryStream::Size() const
{
	return static_cast<int>( m_ByteArray.Size() );
}

const byte* DynamicMemoryStream::GetReadPointer() const
{
	return m_ByteArray.GetData() + m_Offset;
}

int DynamicMemoryStream::GetRemainingSize() const
{
	return m_ByteArray.Size() - m_Offset;
}