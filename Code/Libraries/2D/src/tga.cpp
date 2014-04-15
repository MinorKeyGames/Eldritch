#include "core.h"
#include "tga.h"
#include "idatastream.h"

void TGA::Load( const IDataStream& Stream, TGA::SHeader& InOutHeader )
{
	InOutHeader.m_IDLength = Stream.ReadInt8();
	InOutHeader.m_ColorMapType = Stream.ReadInt8();
	InOutHeader.m_ImageType = Stream.ReadInt8();
	InOutHeader.m_ColorMapIndex = Stream.ReadInt16();
	InOutHeader.m_ColorMapNum = Stream.ReadInt16();
	InOutHeader.m_ColorMapDepth = Stream.ReadInt8();
	InOutHeader.m_OriginX = Stream.ReadInt16();
	InOutHeader.m_OriginY = Stream.ReadInt16();
	InOutHeader.m_Width = Stream.ReadInt16();
	InOutHeader.m_Height = Stream.ReadInt16();
	InOutHeader.m_BPP = Stream.ReadInt8();
	InOutHeader.m_Flags = Stream.ReadInt8();
}