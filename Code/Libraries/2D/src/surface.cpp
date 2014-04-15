#include "core.h"
#include "surface.h"
#include "mathfunc.h"
#include "idatastream.h"
#include "filestream.h"
#include "tga.h"
#include "matrix.h"
#include "mathcore.h"
#include "windowwrapper.h"

#define BYTES 3

static uint		DrawLineColor	= 0;
static Surface*	DrawLineSurface	= NULL;

// TODO: There's a lot of duplicated code in the blit functions here.
// Refactor some of it out into private functions or at least macro it up?

Surface::Surface()
:	m_Width( 0 )
,	m_Height( 0 )
,	m_Stride( 0 )
,	m_Padding( 0 )
#if BUILD_WINDOWS_NO_SDL
,	m_hDC( NULL )
,	m_BMP( NULL )
,	m_OldBMP( NULL )
#endif
#if BUILD_SDL
,	m_Surface( NULL )
#endif
,	m_pPixels( NULL )
,	m_BitmapInfo()
,	m_ViewportLeft( 0 )
,	m_ViewportTop( 0 )
,	m_ViewportRight( 0 )
,	m_ViewportBottom( 0 )
{
	Reset( 1, 1, 24 );
}

Surface::Surface( int Width, int Height )
:	m_Width( 0 )
,	m_Height( 0 )
,	m_Stride( 0 )
,	m_Padding( 0 )
#if BUILD_WINDOWS_NO_SDL
,	m_hDC( NULL )
,	m_BMP( NULL )
,	m_OldBMP( NULL )
#endif
,	m_pPixels( NULL )
,	m_BitmapInfo()
,	m_ViewportLeft( 0 )
,	m_ViewportTop( 0 )
,	m_ViewportRight( 0 )
,	m_ViewportBottom( 0 )
{
	Reset( Width, Height, 24 );
}

Surface::Surface( const IDataStream& Stream, ESurfaceFileType FileType )
:	m_Width( 0 )
,	m_Height( 0 )
,	m_Stride( 0 )
,	m_Padding( 0 )
#if BUILD_WINDOWS_NO_SDL
,	m_hDC( NULL )
,	m_BMP( NULL )
,	m_OldBMP( NULL )
#endif
,	m_pPixels( NULL )
,	m_BitmapInfo()
,	m_ViewportLeft( 0 )
,	m_ViewportTop( 0 )
,	m_ViewportRight( 0 )
,	m_ViewportBottom( 0 )
{
	// Load depending on extension
	if( FileType == ESFT_BMP )
	{
		LoadBMP( Stream );
	}
	else if( FileType == ESFT_TGA )
	{
		LoadTGA( Stream );
	}
	else
	{
		WARNDESC( "Unknown file type constructing Surface." );
	}
}

Surface::~Surface()
{
#if BUILD_WINDOWS_NO_SDL
	DeleteObject( m_BMP );
	DeleteObject( m_OldBMP );
	DeleteDC( m_hDC );
#endif
#if BUILD_SDL
	SDL_FreeSurface( m_Surface );
#endif
}

// TODO: Add meaningful return values
int Surface::LoadBMP( const IDataStream& Stream )
{
	SBitmapFileHeader	BMPFileHeader;
	SBitmapInfoHeader	BMPInfoHeader;

	Stream.Read( sizeof( SBitmapFileHeader ), &BMPFileHeader );
	Stream.Read( sizeof( SBitmapInfoHeader ), &BMPInfoHeader );

	int Width	= BMPInfoHeader.m_Width;
	int Height	= BMPInfoHeader.m_Height;
	int Stride	= ComputeStride( BMPInfoHeader.m_Width, BMPInfoHeader.m_BitCount );

	Reset( Width, Height, BMPInfoHeader.m_BitCount );

#if BUILD_WINDOWS_NO_SDL
	Stream.Read( Stride * Height, m_pPixels );
#endif
#if BUILD_SDL
	// SDL stores surfaces from top to bottom, so flip the rows
	byte* pPixels = m_pPixels + ( ( Height - 1 ) * Stride );
	for( int Row = 0; Row < Height; ++Row, pPixels	-= Stride )
	{
		Stream.Read( Stride, pPixels );
	}
#endif

	return 0;
}

int Surface::LoadTGA( const IDataStream& Stream )
{
	TGA::SHeader Header;
	TGA::Load( Stream, Header );

	ASSERTDESC( Header.m_ImageType == TGA::EIT_RLE_RBG, "Loading this type of TGA has not been implemented." );

	// Bunch of other checks that may not matter; if ID length and color map are present, I can skip them
	ASSERT( Header.m_IDLength == 0 );
	ASSERT( Header.m_ColorMapType == 0 );
	ASSERT( Header.m_ColorMapIndex == 0 );
	ASSERT( Header.m_ColorMapNum == 0 );
	ASSERT( Header.m_ColorMapDepth == 0 );
	ASSERT( Header.m_OriginX == 0 );
	ASSERT( Header.m_OriginY == 0 );
	ASSERT( Header.m_BPP == 24 );
	ASSERT( Header.m_Flags == 0 );

	int Width = Header.m_Width;
	int Height = Header.m_Height;
	int Stride = ( ( Width + 1 ) * 3 ) & 0xffffffc;	// TODO: Generalize to more than 24-bit images

	Reset( Width, Height, 24 );

	// Read RLE data
	int StridePos = 0;
	int HeightPos = 0;
	byte* pImageData = m_pPixels;
	while( HeightPos < Height )
	{
		byte PacketHeader = Stream.ReadInt8();
		byte PacketSize = 1 + ( PacketHeader & 0x7f );
		if( 0 == ( PacketHeader & 0x80 ) )
		{
			// This is a raw packet
			for( int i = 0; i < PacketSize; ++i )
			{
				*pImageData++ = Stream.ReadInt8();
				*pImageData++ = Stream.ReadInt8();
				*pImageData++ = Stream.ReadInt8();

				StridePos += 3;
				int StrideRemaining = Stride - StridePos;
				if( StrideRemaining < 3 )
				{
					pImageData += StrideRemaining;
					StridePos = 0;
					HeightPos++;
				}
			}
		}
		else
		{
			// This is a run packet
			byte B = Stream.ReadInt8();
			byte G = Stream.ReadInt8();
			byte R = Stream.ReadInt8();

			for( int i = 0; i < PacketSize; ++i )
			{
				*pImageData++ = B;
				*pImageData++ = G;
				*pImageData++ = R;

				StridePos += 3;
				int StrideRemaining = Stride - StridePos;
				if( StrideRemaining < 3 )
				{
					pImageData += StrideRemaining;
					StridePos = 0;
					HeightPos++;
				}
			}
		}
	}

	return 0;
}

void Surface::Reset( int Width, int Height, int BitCount )
{
#if BUILD_WINDOWS_NO_SDL
	if( m_BMP )
	{
		SelectObject( m_hDC, m_OldBMP );
		DeleteObject( m_BMP );
	}

	if( !m_hDC )
	{
		m_hDC = CreateCompatibleDC( NULL );	// Could take a window's DC, but this should work for now
	}
#endif

	m_BitmapInfo.m_Header.m_Size					= sizeof( m_BitmapInfo.m_Header );
	m_Width		= m_BitmapInfo.m_Header.m_Width		= Width;
	m_Height	= m_BitmapInfo.m_Header.m_Height	= Height;
	m_BitmapInfo.m_Header.m_Planes					= 1;
	m_BitmapInfo.m_Header.m_BitCount				= static_cast<uint16>( BitCount );
	m_BitmapInfo.m_Header.m_Compression				= 0;	// BI_RGB

	m_Stride	= ComputeStride( m_Width, BitCount );
	m_Padding	= ComputePadding();

#if BUILD_WINDOWS_NO_SDL
	// NOTE: This only supports 24-bit images! 32-bit was added for SDL icon support only.
	BITMAPINFO* const pBitmapInfo = reinterpret_cast<BITMAPINFO*>( &m_BitmapInfo );
	m_BMP		= CreateDIBSection( m_hDC, pBitmapInfo, DIB_RGB_COLORS, (void**)&m_pPixels, NULL, NULL );
	m_OldBMP	= (HBITMAP)SelectObject( m_hDC, m_BMP );
#endif
#if BUILD_SDL
	m_Surface	= SDL_CreateRGBSurface( 0, m_Width, m_Height, BitCount, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 );
	ASSERT( m_Surface );
	ASSERT( !SDL_MUSTLOCK( m_Surface ) );	// Surfaces are always in RAM now, so there's no reason they should need to be locked.
	m_pPixels	= static_cast<byte*>( m_Surface->pixels );
#endif

	m_ViewportLeft		= 0;
	m_ViewportTop		= 0;
	m_ViewportRight		= m_Width;
	m_ViewportBottom	= m_Height;
}

void Surface::SetViewport( int Left, int Top, int Right, int Bottom )
{
	ASSERT( Left >= 0 );
	ASSERT( Right <= m_Width );
	ASSERT( Left < Right );
	ASSERT( Top >= 0 );
	ASSERT( Bottom <= m_Height );
	ASSERT( Top < Bottom );

	m_ViewportLeft		= Left;
	m_ViewportRight		= Right;
	m_ViewportTop		= Top;
	m_ViewportBottom	= Bottom;
}

// Makes the assumption that this Surface is the same size as the window
void Surface::BlitToWindow( Window* const pWindow ) const
{
	ASSERT( pWindow );

#if BUILD_WINDOWS_NO_SDL
	CHECK( BitBlt( pWindow->GetHDC(), 0, 0, m_Width, m_Height, m_hDC, 0, 0, SRCCOPY ) );
#endif
#if BUILD_SDL
	SDL_Window* const	pSDLWindow		= pWindow->GetSDLWindow();
	ASSERT( pSDLWindow );

	SDL_Surface* const	pWindowSurface	= SDL_GetWindowSurface( pSDLWindow );
	ASSERT( pWindowSurface );

	{
		const int Error = SDL_BlitSurface( m_Surface, NULL, pWindowSurface, NULL );
		ASSERT( Error == 0 );
		Unused( Error );
	}

	{
		const int Error = SDL_UpdateWindowSurface( pSDLWindow );
		ASSERT( Error == 0 );
		Unused( Error );
	}
#endif
}

void Surface::SaveBMP( const IDataStream& Stream )
{
	int Size = m_Height * m_Stride;
	m_BitmapInfo.m_Header.m_SizeImage		= Size;
	m_BitmapInfo.m_Header.m_ColorUsed		= 0;
	m_BitmapInfo.m_Header.m_ColorImportant	= 0;
	m_BitmapInfo.m_Header.m_PixelsPerMeterX	= 0;
	m_BitmapInfo.m_Header.m_PixelsPerMeterY	= 0;

	SBitmapFileHeader Header = {0};
	Header.m_Type		= 'MB';
	Header.m_Size		= static_cast<uint>( 54 + Size );
	Header.m_OffsetBits	= 54;

	Stream.Write( sizeof( SBitmapFileHeader ), &Header );
	Stream.Write( sizeof( SBitmapInfoHeader ), &m_BitmapInfo.m_Header );
	Stream.Write( Size, m_pPixels );
}

// This assumes that both Surfaces are the same format and bit depth
/*static*/ void Surface::ScaledBlit( const Surface& Src, Surface& Dest, int Scalar )
{
	PROFILE_FUNCTION;

	ASSERT( Src.GetWidth() * Scalar == Dest.GetWidth() );
	ASSERT( Src.GetHeight() * Scalar == Dest.GetHeight() );

	const byte* SrcPixels = Src.GetPointerAt( 0, 0 );
	int SrcIncrement = Src.GetStride() * 2 - Src.GetPadding();	// Roll forward (padding) to end of line then roll back two lines (stride*2)
	int DestIncrement = Dest.GetStride() + Scalar * BYTES;	// Roll back scalar width plus one line

	for( int SrcY = 0; SrcY < Src.GetHeight(); ++SrcY )
	{
		for( int SrcX = 0; SrcX < Src.GetWidth(); ++SrcX )
		{
			byte B = *SrcPixels++;
			byte G = *SrcPixels++;
			byte R = *SrcPixels++;
			byte* DestPixels = Dest.GetPointerAt( SrcX * Scalar, SrcY * Scalar );
			for( int j = 0; j < Scalar; ++j )
			{
				for( int i = 0; i < Scalar; ++i )
				{
					*DestPixels++ = B;
					*DestPixels++ = G;
					*DestPixels++ = R;
				}
				DestPixels -= DestIncrement;
			}
		}
		SrcPixels -= SrcIncrement;
	}
}

byte* Surface::GetPointerAt( int x, int y )
{
	DEBUGASSERT( x >= 0 );
	DEBUGASSERT( y >= 0 );
	DEBUGASSERT( x < m_Width );
	DEBUGASSERT( y < m_Height );
	return m_pPixels + ( ( m_Height - ( y + 1 ) ) * m_Stride + x * BYTES );
}

const byte* Surface::GetPointerAt( int x, int y ) const
{
	DEBUGASSERT( x >= 0 );
	DEBUGASSERT( y >= 0 );
	DEBUGASSERT( x < m_Width );
	DEBUGASSERT( y < m_Height );
	return m_pPixels + ( ( m_Height - ( y + 1 ) ) * m_Stride + x * BYTES );
}

int Surface::ComputeStride( const int Width, const int BitCount )
{
	// All Surfaces are 24-bit, so:
	// Stride is Width * 3, rounded up to nearest 4
	// i.e., ( Width * 3 ) + ( Width % 4 )
	// Or the fun bitty version:
	if( BitCount == 24 )
	{
		return ( ( Width + 1 ) * 3 ) & 0xffffffc;
	}
	else
	{
		ASSERT( BitCount == 32 );
		return Width * 4;
	}
}

int Surface::ComputePadding()
{
	return ( m_Width % 4 );
}

void Surface::Clear( unsigned int Color )
{
	// TODO: Only clear viewport

	unsigned char* Pixels = m_pPixels;
	unsigned char r = R_FROM_COLOR( Color );
	unsigned char g = G_FROM_COLOR( Color );
	unsigned char b = B_FROM_COLOR( Color );
	int LessHeight = m_Height - 1;
	for( int j = 0; j < LessHeight; ++j )
	{
		for( int i = 0; i < m_Width; ++i )
		{
			*(unsigned int*)Pixels = Color;
			Pixels += BYTES;
		}
		Pixels += m_Padding;
	}
	// Handle last row separately so that bounds aren't overrun at end (because of 24-bit depth)
	for( int i = 0; i < m_Width; ++i )
	{
		*Pixels++ = b;
		*Pixels++ = g;
		*Pixels++ = r;
	}
}

/*static*/ void Surface::Blit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint Flags, uint ColorKey /*= DEFAULT_COLOR_KEY*/, float Alpha /*= 0.5f*/ )
{
	PROFILE_FUNCTION;

	if( Flags & BLIT_ADDITIVE )
	{
		AdditiveBlit( Src, Dest, dx, dy, w, h, sx, sy );
	}
	else if( ( Flags & ( BLIT_MASK | BLIT_BLEND ) ) == ( BLIT_MASK | BLIT_BLEND ) )
	{
		MaskBlendBlit( Src, Dest, dx, dy, w, h, sx, sy, ColorKey );
	}
	else if( ( Flags & ( BLIT_MASK | BLIT_ALPHA ) ) == ( BLIT_MASK | BLIT_ALPHA ) )
	{
		MaskAlphaBlit( Src, Dest, dx, dy, w, h, sx, sy, ColorKey, Alpha );
	}
	else if( Flags & BLIT_MASK )
	{
		MaskBlit( Src, Dest, dx, dy, w, h, sx, sy, ColorKey );
	}
	else if( Flags & BLIT_BLEND )
	{
		BlendBlit( Src, Dest, dx, dy, w, h, sx, sy );
	}
	else if( Flags & BLIT_ALPHA )
	{
		AlphaBlit( Src, Dest, dx, dy, w, h, sx, sy, Alpha );
	}
	else
	{
		StandardBlit( Src, Dest, dx, dy, w, h, sx, sy );
	}
}

/*static*/ bool Surface::ClipRectangles( const Surface& Src, const Surface& Dest, int& DstX, int& DstY, int& W, int& H, int& SrcX, int& SrcY )
{
	if( SrcX < Src.m_ViewportLeft )
	{
		W += ( SrcX - Src.m_ViewportLeft );
		DstX -= ( SrcX - Src.m_ViewportLeft );
		SrcX = Src.m_ViewportLeft;
	}
	if( SrcY < Src.m_ViewportTop )
	{
		H += ( SrcY - Src.m_ViewportTop );
		DstY -= ( SrcY - Src.m_ViewportTop );
		SrcY = Src.m_ViewportTop;
	}

	if( SrcX + W > Src.m_ViewportRight )
	{
		W += ( Src.m_ViewportRight - ( SrcX + W ) );
	}
	if( SrcY + H > Src.m_ViewportBottom )
	{
		H += ( Src.m_ViewportBottom - ( SrcY + H ) );
	}

	if( DstX < Dest.m_ViewportLeft )
	{
		W += ( DstX - Dest.m_ViewportLeft );
		SrcX -= ( DstX - Dest.m_ViewportLeft );
		DstX = Dest.m_ViewportLeft;
	}
	if( DstY < Dest.m_ViewportTop )
	{
		H += ( DstY - Dest.m_ViewportTop );
		SrcY -= ( DstY - Dest.m_ViewportTop );
		DstY = Dest.m_ViewportTop;
	}

	if( DstX + W > Dest.m_ViewportRight )
	{
		W += ( Dest.m_ViewportRight - ( DstX + W ) );
	}
	if( DstY + H > Dest.m_ViewportBottom )
	{
		H += ( Dest.m_ViewportBottom - ( DstY + H ) );
	}

	return W > 0 && H > 0;
}

int Surface::GetWidth() const
{
	return m_Width;
}

int Surface::GetHeight() const
{
	return m_Height;
}

int Surface::GetPadding() const
{
	return m_Padding;
}

int Surface::GetStride() const
{
	return m_Stride;
}

#if BUILD_WINDOWS_NO_SDL
HDC	Surface::GetHDC()
{
	return m_hDC;
}
#endif

void Surface::DrawPoint( int x, int y, uint Color )
{
	if( x < 0 || y < 0 || x >= m_Width || y >= m_Height )
	{
		return;
	}

	unsigned char* Pixels = GetPointerAt( x, y );
	unsigned char r = R_FROM_COLOR( Color );
	unsigned char g = G_FROM_COLOR( Color );
	unsigned char b = B_FROM_COLOR( Color );
	*Pixels++ = b;
	*Pixels++ = g;
	*Pixels = r;
}

uint Surface::GetColorAtPoint( int x, int y ) const
{
	if( x < 0 || y < 0 || x >= m_Width || y >= m_Height )
	{
		return 0;
	}

	const byte* Pixels = GetPointerAt( x, y );
	byte r = *Pixels++;
	byte g = *Pixels++;
	byte b = *Pixels;

	return RGB_TO_COLOR( r, g, b );
}

void DrawLineCallback( int x, int y, bool& Break, void* pContext )
{
	Unused( Break );
	Unused( pContext );

	DrawLineSurface->DrawPoint( x, y, DrawLineColor );
}

void Surface::DrawLine( int x1, int y1, int x2, int y2, uint Color )
{
	DrawLineColor	= Color;
	DrawLineSurface	= this;
	Math::Bresenham( x1, y1, x2, y2, &DrawLineCallback, NULL );
}

void Surface::DrawBox( int x1, int y1, int x2, int y2, uint Color )
{
	if( x1 > x2 )
	{
		int t = x1;
		x1 = x2;
		x2 = t;
	}

	if( y1 > y2 )
	{
		int t = y1;
		y1 = y2;
		y2 = t;
	}

	for( int x = x1; x < x2; ++x )
	{
		DrawPoint( x, y1, Color );
		DrawPoint( x, y2 - 1, Color );
	}
	for( int y = y1; y < y2; ++y )
	{
		DrawPoint( x1, y, Color );
		DrawPoint( x2 - 1, y, Color );
	}
}

void Surface::DrawFilledBox( int x1, int y1, int x2, int y2, uint Color )
{
	if( x1 > x2 )
	{
		int t = x1;
		x1 = x2;
		x2 = t;
	}

	if( y1 > y2 )
	{
		int t = y1;
		y1 = y2;
		y2 = t;
	}

	for( int y = y1; y < y2; ++y )
	{
		for( int x = x1; x < x2; ++x )
		{
			DrawPoint( x, y, Color );
		}
	}
}

/*static*/ void Surface::StandardBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy )
{
	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding();
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding();

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		memcpy( DestPixels, SrcPixels, Length );
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::MaskBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint ColorKey )
{
	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding() + Length;
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding() + Length;

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		for( int i = 0; i < w; ++i )
		{
			// Fixed this when last three bytes can't be converted to a uint because end of block.
			// The other methods need to use this version when/if I ever actually dust this class off.
			byte B = *SrcPixels++;
			byte G = *SrcPixels++;
			byte R = *SrcPixels++;
			if( RGB_TO_COLOR( R, G, B ) == ColorKey )
			{
				DestPixels += BYTES;
			}
			else
			{
				*DestPixels++ = B;
				*DestPixels++ = G;
				*DestPixels++ = R;
			}
		}
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::MaskBlendBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint ColorKey )
{
	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding() + Length;
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding() + Length;

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		for( int i = 0; i < w; ++i )
		{
			// Conditional: bad, but coming up with a *faster* mathy way is hard
			if( ( *(unsigned int*)SrcPixels & 0x00ffffff ) != ColorKey )
			{
				*DestPixels++ = ( *DestPixels + *SrcPixels++ ) >> 1;
				*DestPixels++ = ( *DestPixels + *SrcPixels++ ) >> 1;
				*DestPixels++ = ( *DestPixels + *SrcPixels++ ) >> 1;
			}
			else
			{
				DestPixels += BYTES;
				SrcPixels += BYTES;
			}
		}
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::MaskAlphaBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint ColorKey, float Alpha )
{
	if( Alpha == 0.0f )
	{
		return;
	}

	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	float OneMinusAlpha = 1.f - Alpha;
	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding() + Length;
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding() + Length;

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		for( int i = 0; i < w; ++i )
		{
			// Conditional: bad, but coming up with a *faster* mathy way is hard
			if( ( *(unsigned int*)SrcPixels & 0x00ffffff ) != ColorKey )
			{
				*DestPixels++ = (byte)( *DestPixels * OneMinusAlpha + *SrcPixels++ * Alpha );
				*DestPixels++ = (byte)( *DestPixels * OneMinusAlpha + *SrcPixels++ * Alpha );
				*DestPixels++ = (byte)( *DestPixels * OneMinusAlpha + *SrcPixels++ * Alpha );
			}
			else
			{
				DestPixels += BYTES;
				SrcPixels += BYTES;
			}
		}
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::BlendBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy )
{
	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding() + Length;
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding() + Length;

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		for( int i = 0; i < w; ++i )
		{
			*DestPixels++ = ( *DestPixels + *SrcPixels++ ) >> 1;
			*DestPixels++ = ( *DestPixels + *SrcPixels++ ) >> 1;
			*DestPixels++ = ( *DestPixels + *SrcPixels++ ) >> 1;
		}
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::AlphaBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, float Alpha )
{
	if( Alpha == 0.0f )
	{
		return;
	}

	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	float OneMinusAlpha = 1.f - Alpha;
	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding() + Length;
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding() + Length;

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		for( int i = 0; i < w; ++i )
		{
			*DestPixels++ = (byte)( *DestPixels * OneMinusAlpha + *SrcPixels++ * Alpha );
			*DestPixels++ = (byte)( *DestPixels * OneMinusAlpha + *SrcPixels++ * Alpha );
			*DestPixels++ = (byte)( *DestPixels * OneMinusAlpha + *SrcPixels++ * Alpha );
		}
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::AdditiveBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy )
{
	if( !ClipRectangles( Src, Dest, dx, dy, w, h, sx, sy ) )
	{
		return;
	}

	int Length = w * BYTES;
	int SrcIncrement = ( Src.GetWidth() * BYTES ) + Src.GetPadding() + Length;
	int DestIncrement = ( Dest.GetWidth() * BYTES ) + Dest.GetPadding() + Length;

	byte* DestPixels = Dest.GetPointerAt( dx, dy );
	const byte* SrcPixels = Src.GetPointerAt( sx, sy );

	// Would it be faster to iterate forward (Pixels += ...)? for cache reasons...?
	for( int j = 0; j < h; ++j )
	{
		for( int i = 0; i < w; ++i )
		{
			*DestPixels++ = MMin( *DestPixels + *SrcPixels, 255 );
			++SrcPixels;
			*DestPixels++ = MMin( *DestPixels + *SrcPixels, 255 );
			++SrcPixels;
			*DestPixels++ = MMin( *DestPixels + *SrcPixels, 255 );
			++SrcPixels;
		}
		DestPixels -= DestIncrement;
		SrcPixels -= SrcIncrement;
	}
}

/*static*/ void Surface::TransformedMaskedBlit( const Surface& Src, Surface& Dest, int dx, int dy, float Scale, float RotationRadians, uint ColorKey )
{
	// Use -RotationRadians because +Y is down in screen coords
	Matrix Transform = Matrix::CreateRotationAboutZ( -RotationRadians ) * Matrix::CreateScale( Vector( Scale, Scale, Scale ) ) * Matrix::CreateTranslation( Vector( (float)dx, (float)dy, 0 ) );
	Matrix InvTransform = Transform.GetInverse();

	Vector TopLeft = Vector( 0, 0, 0 ) * Transform;
	Vector TopRight = Vector( (float)Src.GetWidth(), 0, 0 ) * Transform;
	Vector BottomLeft = Vector( 0, (float)Src.GetHeight(), 0 ) * Transform;
	Vector BottomRight = Vector( (float)Src.GetWidth(), (float)Src.GetHeight(), 0 ) * Transform;

	int MinX = (int)( Min( Min( TopLeft.x, TopRight.x ), Min( BottomLeft.x, BottomRight.x ) ) );
	int MinY = (int)( Min( Min( TopLeft.y, TopRight.y ), Min( BottomLeft.y, BottomRight.y ) ) );
	int MaxX = (int)( Max( Max( TopLeft.x, TopRight.x ), Max( BottomLeft.x, BottomRight.x ) ) );
	int MaxY = (int)( Max( Max( TopLeft.y, TopRight.y ), Max( BottomLeft.y, BottomRight.y ) ) );

	// TEMP: Draw a debug rectangle around the borders
	uint ColorWhite = RGB_TO_COLOR( 255, 255, 255 );
	Dest.DrawLine( (int)TopLeft.x, (int)TopLeft.y, (int)TopRight.x, (int)TopRight.y, ColorWhite );
	Dest.DrawLine( (int)TopLeft.x, (int)TopLeft.y, (int)BottomLeft.x, (int)BottomLeft.y, ColorWhite );
	Dest.DrawLine( (int)TopRight.x, (int)TopRight.y, (int)BottomRight.x, (int)BottomRight.y, ColorWhite );
	Dest.DrawLine( (int)BottomLeft.x, (int)BottomLeft.y, (int)BottomRight.x, (int)BottomRight.y, ColorWhite );
	Dest.DrawBox( MinX, MinY, MaxX, MaxY, ColorWhite );

	for( int y = MinY; y < MaxY; ++y )
	{
		for( int x = MinX; x < MaxX; ++x )
		{
			Vector Sample( (float)x, (float)y, 0.f );
			Sample *= InvTransform;

			// TODO: Do masking, clamping, mirroring, or wrapping at edges (for now, mask)
			if( Sample.x < 0.f || Sample.x >= (float)Src.GetWidth() || Sample.y < 0.f || Sample.y >= (float)Src.m_Height )
			{
				continue;
			}

			byte* DestPixels = Dest.GetPointerAt( x, y );
			const byte* SrcPixels = Src.GetPointerAt( (int)Sample.x, (int)Sample.y );

			if( ( *(unsigned int*)SrcPixels & 0x00ffffff ) == ColorKey )
			{
				continue;
			}

			*DestPixels++ = *SrcPixels++;
			*DestPixels++ = *SrcPixels++;
			*DestPixels++ = *SrcPixels++;

			//int SampleX = (int)( )
		}
	}
}