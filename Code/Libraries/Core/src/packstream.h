#ifndef PACKSTREAM_H
#define PACKSTREAM_H

// PackStream replaces PackStream and supports multiple package files
// for superior patching, DLC, etc. Client code can manage the order in which
// package files are checked for matches so that newer packages may override
// content in older packages, e.g., for lightweight content patching.

#include "idatastream.h"
#include "array.h"
#include "simplestring.h"

class FileStream;
class MemoryStream;

struct SPackageFileHeader
{
	SPackageFileHeader();

	uint	m_MagicID;
	uint	m_NumFiles;
	uint	m_OffsetToFiles;
	uint	m_FilesSize;
};

struct SPackageFileEntry
{
	SPackageFileEntry();

	SimpleString	m_Filename;
	uint			m_OffsetToFile;		// Offset from the start of the file block
	uint32			m_CompressedSize;
	uint32			m_DecompressedSize;
	uint32			m_Compressed;		// uint32 instead of bool to remind me that it would be padded out to this size anyway
};

struct SPackageFile
{
	SPackageFile();

	SimpleString*				m_Filename;
	FileStream*					m_Stream;
	SPackageFileHeader			m_Header;
	Array<SPackageFileEntry*>	m_Entries;
};

class PackStream : public IDataStream
{
public:
	PackStream( const char* Filename );
	~PackStream();

	static void	StaticAddPackageFile( const char* PackageFilename, const bool PreemptExistingPackages = false );	// If PreemptExistingPackages, this package will be searched first for a match
	static void StaticShutDown();
	static bool	StaticFileExists( const char* Filename );

	virtual int	Read( int NumBytes, void* Buffer ) const;
	virtual int	Write( int NumBytes, const void* Buffer ) const;	// Not implemented for PackStream
	virtual int PrintF( const char* Str, ... ) const;				// Not implemented for PackStream
	virtual int SetPos( int Position ) const;
	virtual int	GetPos() const;
	virtual int EOS() const;
	virtual int	Size() const;

	// For FMOD streaming
	const char*	GetPhysicalFilename() const;
	uint		GetSubfileOffset() const;
	uint		GetSubfileLength() const;

private:
	// Members of the local stream
	SimpleString				m_Filename;				// The full name of the file that was requested, regardless of packed mode
	SPackageFile*				m_PackageFile;			// Which package file m_MemoryStream is reading from
	SPackageFileEntry*			m_FileEntry;			// Which packed file m_MemoryStream represents
	FileStream*					m_FileStream;			// The non-packed file
	MemoryStream*				m_MemoryStream;			// The packed file decompressed to a buffer
	byte*						m_PackageFileBuffer;	// Buffer for m_MemoryStream
	bool						m_PackedMode;			// Is this stream reading from the decompressed memory stream or a loose file on disk

	// Static members for the global system
	static Array<SPackageFile>	sm_PackageFiles;
};

#endif // PACKSTREAM_H