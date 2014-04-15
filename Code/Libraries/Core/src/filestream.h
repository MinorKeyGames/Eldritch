#ifndef FILESTREAM_H
#define FILESTREAM_H

#include <stdio.h>
#include "idatastream.h"

class FileStream : public IDataStream
{
public:
	enum EFileMode
	{
		EFM_None,
		EFM_Read,
		EFM_Write,
		EFM_Append,
	};

	FileStream( const char* FileName, EFileMode FileMode );
	~FileStream();

	virtual int	Read( int NumBytes, void* Buffer ) const;
	virtual int	Write( int NumBytes, const void* Buffer ) const;
	virtual int PrintF( const char* Str, ... ) const;
	virtual int	SetPos( int Position ) const;
	virtual int	GetPos() const;
	virtual int EOS() const;
	virtual int	Size() const;

private:
	FileStream();

	FILE*		m_TheFile;
	EFileMode	m_FileMode;
};

#endif