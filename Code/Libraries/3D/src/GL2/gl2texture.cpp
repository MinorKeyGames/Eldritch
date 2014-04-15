#include "core.h"
#include "gl2texture.h"
#include "idatastream.h"
#include "configmanager.h"
#include "mathcore.h"

GL2Texture::GL2Texture()
:	m_Texture( 0 )
{
}

GL2Texture::GL2Texture( GLuint Texture )
:	m_Texture( Texture )
{
}

GL2Texture::~GL2Texture()
{
	if( m_Texture != 0 )
	{
		glDeleteTextures( 1, &m_Texture );
	}
}

/*virtual*/ void* GL2Texture::GetHandle()
{
	return &m_Texture;
}

/*virtual*/ void GL2Texture::CreateTexture( byte* const ARGBImage )
{
	XTRACE_FUNCTION;

	glGenTextures( 1, &m_Texture );
	ASSERT( m_Texture != 0 );

	glBindTexture( GL_TEXTURE_2D, m_Texture );

	const int	MipLevels	= CountMipLevels();

	STATICHASH( DebugMips );
	const bool	DebugMips	= ConfigManager::GetBool( sDebugMips );

	// Now fill the texture (and mipmaps)
	int		Width		= m_Width;
	int		Height		= m_Height;
	int		MipWidth	= 0;
	int		MipHeight	= 0;
	byte*	ThisLevel	= ARGBImage;
	byte*	NextLevel	= NULL;
	for( int MipLevel = 0; MipLevel < MipLevels; ++MipLevel )
	{
		glTexImage2D( GL_TEXTURE_2D, MipLevel, GL_RGBA8, Width, Height, 0, GL_BGRA, GL_UNSIGNED_BYTE, ThisLevel );

		if( DebugMips )
		{
			NextLevel = MakeDebugMip( MipLevel + 1, Width, Height, MipWidth, MipHeight );
		}
		else
		{
			NextLevel = MakeMip( Width, Height, MipWidth, MipHeight, ThisLevel );
		}

		SafeDeleteArray( ThisLevel );
		ThisLevel	= NextLevel;
		Width		= MipWidth;
		Height		= MipHeight;
	}
	SafeDeleteArray( NextLevel );
}

// Mirrors DDCOLORKEY
struct SDDColorKey
{
	uint	m_ColorSpaceLow;
	uint	m_ColorSpaceHigh;
};

// Mirrors DDSCAPS2
struct SDDSurfaceCaps
{
	uint	m_Caps[4];
};

// Mirrors DDPIXELFORMAT
struct SDDPixelFormat
{
	uint	m_Size;
	uint	m_Flags;
	uint	m_ID;
	uint	m_BitCount;
	uint	m_BitMasks[4];
};

// Mirrors DDSURFACEDESC2
struct SDDSurfaceFormat
{
	uint			m_Size;
	uint			m_Flags;
	uint			m_Height;
	uint			m_Width;
	int				m_Pitch;
	uint			m_NumBackBuffers;
	uint			m_NumMipMaps;
	uint			m_AlphaBitDepth;
	uint			m_Reserved;
	void*			m_Surface;
	SDDColorKey		m_DestOverlayColorKey;
	SDDColorKey		m_DestBlitColorKey;
	SDDColorKey		m_SrcOverlayColorKey;
	SDDColorKey		m_SrcBlitColorKey;
	SDDPixelFormat	m_PixelFormat;
	SDDSurfaceCaps	m_Caps;
	uint			m_TextureStage;
};

#define DDS_TAG		0x20534444	// 'DDS '
#define DXT1_TAG	0x31545844	// 'DXT1'
#define DXT3_TAG	0x33545844	// 'DXT3'
#define DXT5_TAG	0x35545844	// 'DXT5'

/*virtual*/ void GL2Texture::CreateTextureFromDDS( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	ASSERT( GLEW_EXT_texture_compression_s3tc );

	const uint DDSTag = Stream.ReadUInt32();
	DEVASSERT( DDSTag == DDS_TAG );
	Unused( DDSTag );

	SDDSurfaceFormat DDSFormat;
	Stream.Read( sizeof( SDDSurfaceFormat ), &DDSFormat );
	DEVASSERT( DDSFormat.m_Size == sizeof( SDDSurfaceFormat ) );

	// GL doesn't support DXT2 or DXT4 (premultiplied alpha) formats.
	DEVASSERT( 
		DDSFormat.m_PixelFormat.m_ID == DXT1_TAG ||
		DDSFormat.m_PixelFormat.m_ID == DXT3_TAG ||
		DDSFormat.m_PixelFormat.m_ID == DXT5_TAG );

	m_Width		= DDSFormat.m_Width;
	m_Height	= DDSFormat.m_Height;

	GLenum GLFormat = 0;
	if( DDSFormat.m_PixelFormat.m_ID == DXT1_TAG )
	{
		GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	}
	else if( DDSFormat.m_PixelFormat.m_ID == DXT3_TAG )
	{
		GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	}
	else if( DDSFormat.m_PixelFormat.m_ID == DXT5_TAG )
	{
		GLFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	}

	glGenTextures( 1, &m_Texture );
	ASSERT( m_Texture != 0 );

	glBindTexture( GL_TEXTURE_2D, m_Texture );

	GLsizei		Width		= m_Width;
	GLsizei		Height		= m_Height;
	const uint	FormatBytes	= ( DDSFormat.m_PixelFormat.m_ID == DXT1_TAG ? 8 : 16 );
	const uint	MinSize		= 1;
	const uint	BlocksWide	= Max( MinSize, m_Width >> 2 );
	const uint	BlocksHigh	= Max( MinSize, m_Height >> 2 );
	uint		ReadBytes	= BlocksWide * BlocksHigh * FormatBytes;
	Array<byte>	ReadArray;
	ReadArray.SetDeflate( false );
	ReadArray.Reserve( ReadBytes );
	for( uint MipLevel = 0; MipLevel < DDSFormat.m_NumMipMaps; ++MipLevel )
	{
		ReadArray.Resize( ReadBytes );
		Stream.Read( ReadBytes, ReadArray.GetData() );

		glCompressedTexImage2D( GL_TEXTURE_2D, MipLevel, GLFormat, Width, Height, 0, ReadBytes, ReadArray.GetData() );

		Width	= Max( 1, Width >> 1 );
		Height	= Max( 1, Height >> 1 );
		ReadBytes = Max( FormatBytes, ReadBytes >> 2 );
	}
}