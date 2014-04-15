#ifndef TEXTURECOMMON_H
#define TEXTURECOMMON_H

#include "itexture.h"
#include "3d.h"

class IDataStream;

class TextureCommon : public ITexture
{
public:
	TextureCommon();
	virtual ~TextureCommon();

	virtual uint	GetWidth() const { return m_Width; }
	virtual uint	GetHeight() const { return m_Height; }

	void			Initialize( const char* const Filename );

protected:
	int				CountMipLevels();

	void			InitializeFromFile( const char* const Filename, byte*& OutARGBImage );
	void			LoadBMP( const IDataStream& Stream, int& Width, int& Height, byte*& ARGBImage );
	void			LoadTGA( const IDataStream& Stream, int& Width, int& Height, byte*& ARGBImage );

	virtual void	CreateTexture( byte* const ARGBImage ) = 0;
	virtual void	CreateTextureFromDDS( const IDataStream& Stream ) = 0;

	unsigned char*	ConvertRGBtoARGB( int Width, int Height, unsigned char* Image );
	unsigned char*	MakeMip( int Width, int Height, int& MipWidth, int& MipHeight, unsigned char* Image );	// Makes the next smaller mip for the given texture--delete manually after use!
	unsigned char*	MakeDebugMip( int MipLevel, int Width, int Height, int& MipWidth, int& MipHeight );

	uint	m_Width;
	uint	m_Height;
};

#endif // TEXTURECOMMON_H