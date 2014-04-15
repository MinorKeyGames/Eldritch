#ifndef TGA_H
#define TGA_H

// Image types:
// 0  -  No image data included.
// 1  -  Uncompressed, color-mapped images.
// 2  -  Uncompressed, RGB images.
// 3  -  Uncompressed, black and white images.
// 9  -  Runlength encoded color-mapped images.
// 10  -  Runlength encoded RGB images.
// 11  -  Compressed, black and white images.
// 32  -  Compressed color-mapped data, using Huffman, Delta, and runlength encoding.
// 33  -  Compressed color-mapped data, using Huffman, Delta, and runlength encoding.  4-pass quadtree-type process.

class IDataStream;

namespace TGA
{
	enum EImageType
	{
		EIT_None = 0,
		EIT_Unc_Map = 1,
		EIT_Unc_RGB = 2,
		EIT_Unc_BW = 3,
		EIT_RLE_Map = 9,
		EIT_RLE_RBG = 10,
		EIT_Cmp_BW = 11,
		EIT_Cmp_Map = 32,
		EIT_Cmp_Map4 = 33,
	};

	struct SHeader
	{
		SHeader()
		:	m_IDLength( 0 )
		,	m_ColorMapType( 0 )
		,	m_ImageType( 0 )
		,	m_ColorMapIndex( 0 )
		,	m_ColorMapNum( 0 )
		,	m_ColorMapDepth( 0 )
		,	m_OriginX( 0 )
		,	m_OriginY( 0 )
		,	m_Width( 0 )
		,	m_Height( 0 )
		,	m_BPP( 0 )
		,	m_Flags( 0 )
		{
		}

		int8	m_IDLength;
		int8	m_ColorMapType;
		int8	m_ImageType;		// One of EImageType
		int16	m_ColorMapIndex;
		int16	m_ColorMapNum;
		int8	m_ColorMapDepth;
		int16	m_OriginX;
		int16	m_OriginY;
		int16	m_Width;
		int16	m_Height;
		int8	m_BPP;
		int8	m_Flags;
	};

	void Load( const IDataStream& Stream, SHeader& InOutHeader );
}

#endif // TGA_H