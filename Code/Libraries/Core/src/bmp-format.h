#ifndef BMPFORMAT_H
#define BMPFORMAT_H

// Mirrors RGBQUAD
struct SRGBQuad
{
	byte	m_Blue;
	byte	m_Green;
	byte	m_Red;
	byte	m_Padding;
};

// Mirrors BITMAPINFOHEADER
struct SBitmapInfoHeader
{
	uint	m_Size;
	int		m_Width;
	int		m_Height;
	uint16	m_Planes;
	uint16	m_BitCount;
	uint	m_Compression;
	uint	m_SizeImage;
	int		m_PixelsPerMeterX;
	int		m_PixelsPerMeterY;
	uint	m_ColorUsed;
	uint	m_ColorImportant;
};

// Mirrors BITMAPINFO
struct SBitmapInfo
{
	SBitmapInfoHeader	m_Header;
	SRGBQuad			m_Colors;
};

// Mirrors BITMAPFILEHEADER
// 2-byte aligned
#pragma pack(push, 2)
struct SBitmapFileHeader
{
	uint16	m_Type;
	uint	m_Size;
	uint16	m_Reserved1;
	uint16	m_Reserved2;
	uint	m_OffsetBits;
};
#pragma pack(pop)

#endif // BMPFORMAT_H