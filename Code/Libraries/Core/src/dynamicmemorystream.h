#ifndef DYNAMICMEMORYSTREAM_H
#define DYNAMICMEMORYSTREAM_H

// Intended mainly for writing to a growable array, whereas MemoryStream uses a fixed range.

#include "idatastream.h"
#include "array.h"

class DynamicMemoryStream : public IDataStream
{
public:
	DynamicMemoryStream();

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

	const Array<byte>&		GetArray() const { return m_ByteArray; }

private:
	mutable Array<byte>		m_ByteArray;
	mutable uint			m_Offset;
};

#endif // DYNAMICMEMORYSTREAM_H