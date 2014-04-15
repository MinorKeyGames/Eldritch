#ifndef BUFFEREDFILESTREAM_H
#define BUFFEREDFILESTREAM_H

#include "idatastream.h"
#include "array.h"

class BufferedFileStream : public IDataStream
{
public:
	enum EFileMode
	{
		EFM_None,
		EFM_Read,
		EFM_Write,
		EFM_Append,
	};

	BufferedFileStream( const char* Filename, EFileMode FileMode );
	~BufferedFileStream();

	virtual int	Read( int NumBytes, void* Buffer ) const;
	virtual int	Write( int NumBytes, const void* Buffer ) const;
	virtual int PrintF( const char* Str, ... ) const;
	virtual int	SetPos( int Position ) const;
	virtual int	GetPos() const;
	virtual int EOS() const;
	virtual int	Size() const;

private:
	BufferedFileStream();

	const char*				m_Filename;	// Only used for writing
	mutable Array< byte >	m_Buffer;
	EFileMode				m_FileMode;
	mutable int				m_Position;
	mutable int				m_Filesize;
};

#endif // BUFFEREDFILESTREAM_H