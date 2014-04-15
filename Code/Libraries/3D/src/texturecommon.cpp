#include "core.h"
#include "texturecommon.h"
#include "packstream.h"
#include "bmp-format.h"

#include <string.h>

#define EXT_BMP '\0pmb'
#define EXT_TGA '\0agt'
#define EXT_DDS '\0sdd'

struct TGAHeader
{
	byte	m_SizeOfIDField;
	byte	m_ColorMapType;
	byte	m_ImageType;		// I'll be concerned with 2 and 10 only

	// Broken up into bytes because of word alignment issues
	byte	m_ColorMapOriginLo;
	byte	m_ColorMapOriginHi;
	byte	m_ColorMapLengthLo;
	byte	m_ColorMapLengthHi;
	byte	m_ColorMapBitDepth;

	uint16	m_OriginX;
	uint16	m_OriginY;
	uint16	m_Width;
	uint16	m_Height;
	byte	m_BitDepth;
	byte	m_ImageDescriptor;	// See TGA documentation
};

TextureCommon::TextureCommon()
:	m_Width( 0 )
,	m_Height( 0 )
{
}

void TextureCommon::Initialize( const char* const Filename )
{
	byte* ARGBImage = NULL;
	InitializeFromFile( Filename, ARGBImage );

	if( ARGBImage == NULL )
	{
		// This typically means we loaded a DDS.
		return;
	}

	CreateTexture( ARGBImage );
}

void TextureCommon::InitializeFromFile( const char* const Filename, byte*& OutARGBImage )
{
	XTRACE_FUNCTION;

	// Get the file extension
	size_t			ExtOffset	= strlen( Filename ) - 3;
	unsigned int	Ext			= *(unsigned int*)( Filename + ExtOffset );

	OutARGBImage	= NULL;
	int Width		= 0;
	int Height		= 0;

	// Load depending on extension
	if( Ext == EXT_DDS )
	{
		CreateTextureFromDDS( PackStream( Filename ) );
		return;
	}
	else if( Ext == EXT_BMP )
	{
		LoadBMP( PackStream( Filename ), Width, Height, OutARGBImage );
	}
	else if( Ext == EXT_TGA )
	{
		LoadTGA( PackStream( Filename ), Width, Height, OutARGBImage );
	}

	ASSERT( OutARGBImage );

	m_Width		= Width;
	m_Height	= Height;
}

TextureCommon::~TextureCommon()
{
}

// Load a 24-bit BMP into a 32-bit (ARGB) buffer, creating the necessary mipmaps
void TextureCommon::LoadBMP( const IDataStream& Stream, int& Width, int& Height, byte*& ARGBImage )
{
	XTRACE_FUNCTION;

	SBitmapFileHeader	BMPFileHeader;
	SBitmapInfoHeader	BMPInfoHeader;

	Stream.Read( sizeof( SBitmapFileHeader ), &BMPFileHeader );
	Stream.Read( sizeof( SBitmapInfoHeader ), &BMPInfoHeader );

	Width				= BMPInfoHeader.m_Width;
	Height				= BMPInfoHeader.m_Height;
	int Stride			= ( ( Width + 1 ) * 3 ) & 0xffffffc;
	uint8* ImageData	= new uint8[ Stride * Height ];
	uint8* Dest			= ImageData + ( Stride * ( Height - 1 ) );

	// Flip BMP (stored bottom-to-top)
	for( int i = 0; i < Height; ++i )
	{
		Stream.Read( Stride, Dest );
		Dest -= Stride;
	}

	ARGBImage = ConvertRGBtoARGB( Width, Height, ImageData );
	SafeDeleteArray( ImageData );
}

// Load a TGA into a 32-bit (ARGB) buffer, creating the necessary mipmaps
// Should support 24- and 32-bit TGAs, uncompressed and RLE
void TextureCommon::LoadTGA( const IDataStream& Stream, int& Width, int& Height, byte*& ARGBImage )
{
	XTRACE_FUNCTION;

	TGAHeader Header;

	Stream.Read( sizeof( TGAHeader ), &Header );
	Stream.SetPos( Stream.GetPos() + Header.m_SizeOfIDField );

	if( Header.m_ColorMapType )
	{
		uint16 Length = Header.m_ColorMapLengthLo + ( Header.m_ColorMapLengthHi << 8 );
		Stream.SetPos( Stream.GetPos() + ( Length * Header.m_ColorMapBitDepth ) );
	}

	Width = Header.m_Width;
	Height = Header.m_Height;
	int Stride = Width * 4;
	int ImageSize = Stride * Height;
	ARGBImage = new byte[ ImageSize ];

	ASSERT( Header.m_ImageType == 2 || Header.m_ImageType == 10 );
	if( Header.m_ImageType == 2 )
	{
		byte* DestPixels = ARGBImage + Stride * ( Height - 1 );	// Initialize read pointer to the last row, since TGAs are ordered bottom-to-top
		if( Header.m_BitDepth == 24 )
		{
			for( int y = 0; y < Height; ++y )
			{
				for( int x = 0; x < Width; ++x )
				{
					Stream.Read( 3, DestPixels );
					DestPixels += 3;
					*DestPixels++ = 0xff;	// Set the alpha byte
				}
				DestPixels -= Stride * 2;
			}
		}
		else if( Header.m_BitDepth == 32 )
		{
			for( int i = 0; i < Height; ++i )
			{
				Stream.Read( Stride, DestPixels );
				DestPixels -= Stride;
			}
		}
		else
		{
			WARNDESC( "LoadTGA: Unsupported bit depth." );
		}
	}
	else if( Header.m_ImageType == 10 )
	{
		{
			PROFILE_SCOPE( TGA_RLE_Unpack );

			byte* DestPixels = ARGBImage;	// Initialize read pointer to the first row, read upside down, then flip
			if( Header.m_BitDepth == 24 )
			{
				const uint NumPixels = Width * Height;
				uint ReadPixels = 0;

				while( ReadPixels < NumPixels )
				{
					byte PacketHeader = Stream.ReadUInt8();
					uint PacketSize = static_cast<uint>( PacketHeader & 0x7f ) + 1;

					if( PacketHeader & 0x80 )	// RLE packet
					{
						uint RLEValue = 0xffffffff;
						Stream.Read( 3, &RLEValue );
						const byte* const EndDestPixels = DestPixels + PacketSize * 4;
						for( ; DestPixels < EndDestPixels; DestPixels += 4 )
						{
							uint* const DestPixels32 = reinterpret_cast<uint*>( DestPixels );
							*DestPixels32 = RLEValue;
						}
					}
					else						// Raw packet
					{
						const byte* const EndDestPixels = DestPixels + PacketSize * 4;
						for( ; DestPixels < EndDestPixels; )
						{
							Stream.Read( 3, DestPixels );
							DestPixels += 3;
							*DestPixels++ = 0xff;	// Set the alpha byte
						}
					}

					ReadPixels += PacketSize;
				}
			}
			else if( Header.m_BitDepth == 32 )
			{
				const uint NumPixels = Width * Height;
				uint ReadPixels = 0;

				while( ReadPixels < NumPixels )
				{
					byte PacketHeader = Stream.ReadUInt8();
					uint PacketSize = static_cast<uint>( PacketHeader & 0x7f ) + 1;

					if( PacketHeader & 0x80 )	// RLE packet
					{
						uint RLEValue = Stream.ReadUInt32();
						const byte* const EndDestPixels = DestPixels + PacketSize * 4;
						for( ; DestPixels < EndDestPixels; DestPixels += 4 )
						{
							uint* const DestPixels32 = reinterpret_cast<uint*>( DestPixels );
							*DestPixels32 = RLEValue;
						}
					}
					else						// Raw packet
					{
						Stream.Read( PacketSize * 4, DestPixels );
						DestPixels += PacketSize * 4;
					}

					ReadPixels += PacketSize;
				}
			}
			else
			{
				WARNDESC( "LoadTGA: Unsupported bit depth." );
			}
		}

		{
			PROFILE_SCOPE( TGA_RLE_Flip );

			// Flip image vertically
			byte* RowT = new byte[ Stride ];
			const int HalfHeight = Height / 2;
			for( int Row = 0; Row < HalfHeight; ++Row )
			{
				byte* const RowA = ARGBImage + Stride * Row;
				byte* const RowB = ARGBImage + Stride * ( Height - Row - 1 );
				memcpy( RowT, RowA, Stride );
				memcpy( RowA, RowB, Stride );
				memcpy( RowB, RowT, Stride );
			}
			SafeDeleteArray( RowT );
		}
	}
	else
	{
		WARNDESC( "LoadTGA: Unsupported image type." );
	}
}

int TextureCommon::CountMipLevels()
{
	int Width		= m_Width;
	int Height		= m_Height;
	int MipLevels	= 0;

	while( Width || Height )
	{
		Width	>>= 1;
		Height	>>= 1;
		++MipLevels;
	}

	return MipLevels;
}

// Parameter Image is a 24-bit (RGB) image, return value points to a 32-bit (ARGB) image
// WARNING: ALLOCATES NEW DATA
unsigned char* TextureCommon::ConvertRGBtoARGB( int Width, int Height, unsigned char* Image )
{
	uint8* NewImage = new uint8[ Width * Height * 4 ];
	uint8* DestPixels = NewImage;
	uint8* SrcPixels = Image;
	int Padding = Width % 4;
	for( int y = 0; y < Height; ++y )
	{
		for( int x = 0; x < Width; ++x )
		{
			*DestPixels++ = *SrcPixels++;
			*DestPixels++ = *SrcPixels++;
			*DestPixels++ = *SrcPixels++;
			*DestPixels++ = 0xff;
		}
		SrcPixels += Padding;
	}
	return NewImage;
}

// Parameter Image is a 32-bit (ARGB) image, return value points to a 32-bit (ARGB) image
// Doing a simple box filter--this looks mad slow
// WARNING: ALLOCATES NEW DATA
unsigned char* TextureCommon::MakeMip( int Width, int Height, int& MipWidth, int& MipHeight, unsigned char* Image )
{
	if( Width == 1 && Height == 1 )
	{
		return NULL;
	}

	MipWidth = Width >> 1;
	if( MipWidth == 0 )
	{
		MipWidth = 1;
	}

	MipHeight = Height >> 1;
	if( MipHeight == 0 )
	{
		MipHeight = 1;
	}

	uint8* NewMip = new uint8[ 4 * MipWidth * MipHeight ];
	uint8* DestPixels = NewMip;

	uint16 SrcSum;
	byte* SrcBoxTopLeft;
	byte* SrcBoxTopRight;
	byte* SrcBoxBottomLeft;
	byte* SrcBoxBottomRight;
	int SRowO = 4 * Width;
	int SColO = 4;

	for( int y = 0; y < MipHeight; ++y )
	{
		for( int x = 0; x < MipWidth; ++x )
		{
			SrcBoxBottomLeft	= Image + ( 4 * ( ( x << 1 ) + ( ( y << 1 ) * Width ) ) );
			SrcBoxBottomRight	= SrcBoxBottomLeft + SColO;
			SrcBoxTopLeft		= SrcBoxBottomLeft + SRowO;
			SrcBoxTopRight		= SrcBoxTopLeft + SColO;
			for( int c = 0; c < 4; ++c )
			{
				SrcSum = *SrcBoxBottomLeft + *SrcBoxBottomRight + *SrcBoxTopLeft + *SrcBoxTopRight;
				*DestPixels++ = (byte)( SrcSum >> 2 );

				++SrcBoxBottomLeft;
				++SrcBoxBottomRight;
				++SrcBoxTopLeft;
				++SrcBoxTopRight;
			}
		}
	}

	return NewMip;
}

unsigned char* TextureCommon::MakeDebugMip( int MipLevel, int Width, int Height, int& MipWidth, int& MipHeight )
{
	if( Width == 1 && Height == 1 )
	{
		return NULL;
	}

	MipWidth = Width >> 1;
	if( MipWidth == 0 )
	{
		MipWidth = 1;
	}

	MipHeight = Height >> 1;
	if( MipHeight == 0 )
	{
		MipHeight = 1;
	}

	uint8* NewMip = new uint8[ 4 * MipWidth * MipHeight ];
	uint8* DestPixels = NewMip;

	uint8 Blue = 0xff * (uint8)( MipLevel & 1 );
	uint8 Green = 0xff * (uint8)( MipLevel & 2 );
	uint8 Red = 0xff * (uint8)( MipLevel & 4 );

	for( int y = 0; y < MipHeight; ++y )
	{
		for( int x = 0; x < MipWidth; ++x )
		{
			*DestPixels++ = Blue;
			*DestPixels++ = Green;
			*DestPixels++ = Red;
			*DestPixels++ = 0xff;
		}
	}

	return NewMip;
}