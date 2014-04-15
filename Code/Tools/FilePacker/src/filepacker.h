#ifndef FILEPACKER_H
#define FILEPACKER_H

#include "packstream.h"
#include "array.h"

class IDataStream;

class FilePacker
{
public:
	FilePacker();

	int		PackFile( const char* InFilename, const char* PackFilename, bool Compress );
	int		UnpackFile( const char* PackFilename );

	void	ReadInFile( const IDataStream& Stream );
	void	ReadPackFile( const IDataStream& Stream );
	void	AddInFile( const char* InFilename, bool Compress );
	void	WritePackFile( const IDataStream& Stream );
	void	WriteUnpackedFile( const IDataStream& Stream, SPackageFileEntry& FileEntry );

	uint	GetFilesOffset();

	uint	m_InFileSize;
	byte*	m_InFileBuffer;

	uint	m_InFileCompressionSize;	// The size of the whole buffer before compression; the actual compressed size after
	byte*	m_InFileCompressionBuffer;

	SPackageFileHeader			m_Header;
	Array< SPackageFileEntry >	m_FileEntries;
	uint						m_ExistingPackFileSize;
	byte*						m_ExistingPackFileBuffer;	// The blob of compressed data read in from package file
};

#endif // FILEPACKER_H