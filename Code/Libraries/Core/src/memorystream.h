#ifndef MEMORYSTREAM_H
#define MEMORYSTREAM_H

#include "idatastream.h"

class MemoryStream : public IDataStream
{
private:
	MemoryStream();

	byte*			m_Data;
	uint			m_Size;

	mutable uint	m_Offset;

public:
	// If Size == 0, no bounds checking will be done
	MemoryStream( void* pData, uint Size );

	virtual int	Read( int NumBytes, void* Buffer ) const;
	virtual int	Write( int NumBytes, const void* Buffer ) const;
	virtual int PrintF( const char* Str, ... ) const;
	virtual int SetPos( int Position ) const;
	virtual int	GetPos() const;
	virtual int EOS() const;
	virtual int	Size() const;

	// Not part of IDataStream, but something we can do with memory.
	const byte*	GetReadPointer() const;
	int			GetRemainingSize() const;
};

#endif // MEMORYSTREAM_H