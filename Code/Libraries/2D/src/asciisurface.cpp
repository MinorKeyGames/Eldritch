#include "core.h"
#include "asciisurface.h"
#include "mathfunc.h"
#include "asciirenderer.h"
#include "mathcore.h"

static uint8			DrawLineChar	= 0;
static uint8			DrawLineColor	= 0;
static uint				DrawLineFlags	= 0;
static ASCIISurface*	DrawLineSurface	= NULL;
void ASCIISurfaceDrawLineCallback( int x, int y, bool& Break, void* pContext );

ASCIISurface::ASCIISurface( uint w /*= 80*/, uint h /*=25*/ )
:	m_Surface( NULL )
{
	Init( w, h );
}

ASCIISurface::~ASCIISurface()
{
	SafeDeleteArray( m_Surface );
}

void ASCIISurface::Init( uint w /*= 80*/, uint h /*= 25*/, uint8 ch /*= ' '*/, uint8 color /*= 0*/ )
{
	SafeDeleteArray( m_Surface );

	m_Surface = new CHAR_INFO[ w * h ];
	m_SurfaceSize.X = (short)w;
	m_SurfaceSize.Y = (short)h;
	Fill( ch, color );
}

void ASCIISurface::Fill( uint8 ch, uint8 color )
{
	for( int i = 0; i < m_SurfaceSize.X * m_SurfaceSize.Y; ++i )
	{
		m_Surface[i].Char.AsciiChar = ch;
		m_Surface[i].Attributes = color;
	}
}

void ASCIISurface::FillRect( uint8 ch, uint8 color, int x, int y, int w, int h )
{
	if( !CheckBounds( x, y ) ) return;
	if( !CheckBounds( x + w - 1, y + h - 1 ) ) return;

	int yBound = y + h;
	for( int j = y; j < yBound; ++j )
	{
		int Offset = x + j * m_SurfaceSize.X;
		for( int i = 0; i < w; ++i, ++Offset )
		{
			m_Surface[ Offset ].Char.AsciiChar = ch;
			m_Surface[ Offset ].Attributes = color;
		}
	}
}

void ASCIISurface::SetChar( uint8 ch, uint8 color, int x, int y, int Flags /*= 0*/ )
{
	if( !CheckBounds( x, y ) ) return;

	if( Flags & ASURF_FG )
	{
		color = ( color & FG_MASK ) | ( GetColor( x, y ) & BG_MASK );
	}
	else if( Flags & ASURF_BG )
	{
		color = ( color & BG_MASK ) | ( GetColor( x, y ) & FG_MASK );
	}

	GetCharAt( x, y ).Char.AsciiChar = ch;
	GetCharAt( x, y ).Attributes = color;
}

void ASCIISurface::SetChar( uint8 ch, int x, int y )
{
	if( !CheckBounds( x, y ) ) return;
	GetCharAt( x, y ).Char.AsciiChar = ch;
}

void ASCIISurface::SetColor( uint8 color, int x, int y, int Flags /*= 0*/ )
{
	if( !CheckBounds( x, y ) ) return;

	if( Flags & ASURF_FG )
	{
		color = ( color & FG_MASK ) | ( GetColor( x, y ) & BG_MASK );
	}
	else if( Flags & ASURF_BG )
	{
		color = ( color & BG_MASK ) | ( GetColor( x, y ) & FG_MASK );
	}

	GetCharAt( x, y ).Attributes = color;
}

void ASCIISurface::SetChar( CHAR_INFO ch, int x, int y, int Flags /*= 0*/ )
{
	if( !CheckBounds( x, y ) ) return;

	if( Flags & ASURF_FG )
	{
		ch.Attributes = ( ch.Attributes & FG_MASK ) | ( GetColor( x, y ) & BG_MASK );
	}
	else if( Flags & ASURF_BG )
	{
		ch.Attributes = ( ch.Attributes & BG_MASK ) | ( GetColor( x, y ) & FG_MASK );
	}

	GetCharAt( x, y ) = ch;
}

void ASCIISurface::SetColorChar( uint16 ch, int x, int y )
{
	if( !CheckBounds( x, y ) ) return;

	GetCharAt( x, y ).Char.AsciiChar = ( uint8 )( ( ch & 0xff00 ) >> 8 );
	GetCharAt( x, y ).Attributes = ( ch & 0xff );
}

uint8 ASCIISurface::GetChar( int x, int y ) const
{
	if( !CheckBounds( x, y ) )
	{
		return 0;
	}

	return GetCharAt( x, y ).Char.AsciiChar;
}

uint8 ASCIISurface::GetColor( int x, int y ) const
{
	if( !CheckBounds( x, y ) )
	{
		return 0;
	}

	return ( uint8 )GetCharAt( x, y ).Attributes;
}

void ASCIISurface::Print( const char * str, uint8 color, int x, int y, int Flags /*= 0*/ )
{
	Print( str, 0, color, x, y, Flags );
}

void ASCIISurface::PrintCentered( const char* String, uint8 Color, int Y, int XBase /*= 0*/, int XSpan /*= 0*/, int Flags /*= 0*/ )
{
	if( XSpan <= 0 )
	{
		XSpan = m_SurfaceSize.X;
	}

	uint Length = ( uint )strlen( String );
	int X = XBase + ( ( XSpan - Length ) / 2 );

	Print( String, Length, Color, X, Y, Flags );
}

void ASCIISurface::Print(const char * str, uint MaxChars, uint8 color, int x, int y, int Flags /*= 0*/ )
{
	uint Length = ( uint )strlen( str );
	MaxChars = MaxChars > 0 ? Min( MaxChars, Length ) : Length;

	int myX = x;
	int myY = y;

	for( uint i = 0; i < MaxChars; ++i )
	{
		if( str[i] == '\n' )
		{
			myX = x;
			myY++;
		}
		else
		{
			SetChar( str[i], color, myX, myY, Flags );

			if( ++myX == m_SurfaceSize.X )
			{
				myX = x;
				myY++;
			}
		}
	}
}

void ASCIISurface::DrawLine( int StartX, int StartY, int EndX, int EndY, uint8 ch, uint8 color, int Flags /*= 0*/ )
{
	DrawLineChar = ch;
	DrawLineColor = color;
	DrawLineSurface = this;
	DrawLineFlags = Flags;
	Math::Bresenham( StartX, StartY, EndX, EndY, ASCIISurfaceDrawLineCallback, NULL );
}

void ASCIISurfaceDrawLineCallback( int x, int y, bool& Break, void* pContext )
{
	Unused( Break );
	Unused( pContext );
	DrawLineSurface->SetChar( DrawLineChar, DrawLineColor, x, y, DrawLineFlags );
}

void ASCIISurface::ABlit(const ASCIISurface &src, ASCIISurface &dst, uint16 srcX, uint16 srcY, uint16 srcW, uint16 srcH, int16 dstX, int16 dstY, int Flags /*= 0*/, bool masked, uint8 maskChar)
{
	PROFILE_FUNCTION;

	// Constrain to bounds
	if( srcW == 0 ) srcW = src.GetSurfSize().X;
	if( srcH == 0 ) srcH = src.GetSurfSize().Y;
	if( srcX + srcW > src.GetSurfSize().X ) srcW = src.GetSurfSize().X - srcX;
	if( srcY + srcH > src.GetSurfSize().Y ) srcH = src.GetSurfSize().Y - srcY;

	int origSrcW = src.GetSurfSize().X;

	// These are the dimensions that will be copied
	int16 w = MMin( srcW, dst.GetSurfSize().X - dstX );
	int16 h = MMin( srcH, dst.GetSurfSize().Y - dstY );

	CHAR_INFO ch;
	for( int y = 0; y < h; ++y )
	{
		for( int x = 0; x < w; ++x )
		{
			ch = src.GetSurfData()[ srcX + x + (srcY + y) * origSrcW ];
			if( !masked || ch.Char.AsciiChar != maskChar )
			{
				dst.SetChar( ch, dstX + x, dstY + y, Flags );
			}
		}
	}
}

bool ASCIISurface::CheckBounds( int x, int y ) const
{
	return ( x >= 0 && x < m_SurfaceSize.X && y >= 0 && y < m_SurfaceSize.Y );
}

// Always CheckBounds before calling!
CHAR_INFO& ASCIISurface::GetCharAt( int x, int y ) const
{
	return m_Surface[ x + y * m_SurfaceSize.X ];
}