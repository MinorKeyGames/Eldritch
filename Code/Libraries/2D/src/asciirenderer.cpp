#include "core.h"
#include "asciirenderer.h"
#include "asciisurface.h"
#include "mathcore.h"

uint8 GetBitWiseColor( uint8 fore_r, uint8 fore_g, uint8 fore_b, uint8 fore_i, uint8 back_r, uint8 back_g, uint8 back_b, uint8 back_i )
{
	return (fore_r << 2 | fore_g << 1 | fore_b | fore_i << 3 | back_r << 6 | back_g << 5 | back_b << 4 | back_i << 7);
}

ASCIIRenderer::ASCIIRenderer()
:	m_hConsoleOutput( NULL )
,	m_WindowRect()
,	m_FlickerFreeBuffer( NULL )
,	m_Width( 0 )
,	m_Height( 0 )
{
	m_WindowRect.Left	= 0;
	m_WindowRect.Top	= 0;
	m_WindowRect.Right	= 0;
	m_WindowRect.Bottom	= 0;
}

ASCIIRenderer::~ASCIIRenderer()
{
	SafeDelete( m_FlickerFreeBuffer );
}

void ASCIIRenderer::Init( uint width /*= 80*/, uint height /*= 25*/ )
{
	m_hConsoleOutput = GetStdHandle( STD_OUTPUT_HANDLE );
	ASSERT( m_hConsoleOutput );

	m_Width = width;
	m_Height = height;

	// In order to resize the screen buffer (potentially smaller than the
	// default window), we need to set the window to zero dimensions.
	SetConsoleWindowInfo( m_hConsoleOutput, true, &m_WindowRect );

	m_WindowRect.Left	= 0;
	m_WindowRect.Top	= 0;
	m_WindowRect.Right	= (short)( width - 1 );
	m_WindowRect.Bottom	= (short)( height - 1 );

	COORD BufferSize;
	BufferSize.X = ( short )width;
	BufferSize.Y = ( short )height;

	SetConsoleScreenBufferSize( m_hConsoleOutput, BufferSize );		// Next, set the screen buffer size, which cannot be smaller in either dimension than the window.
	SetConsoleWindowInfo( m_hConsoleOutput, true, &m_WindowRect );	// Finally, set the window size, which cannot be larger in either dimension than the buffer.

	ShowCursor( false );
}

void ASCIIRenderer::Draw( const ASCIISurface &surface )
{
	PROFILE_FUNCTION;

	ASSERT( m_hConsoleOutput );
	ASSERT( surface.GetSurfSize().X == m_Width );
	ASSERT( surface.GetSurfSize().Y == m_Height );

	COORD startLocation = {0, 0};

	if( m_FlickerFreeBuffer )
	{
		ASSERT( surface.GetSurfSize().X == m_FlickerFreeBuffer->GetSurfSize().X );
		ASSERT( surface.GetSurfSize().Y == m_FlickerFreeBuffer->GetSurfSize().Y );

		ReadConsoleOutput( m_hConsoleOutput, m_FlickerFreeBuffer->GetSurfData(), m_FlickerFreeBuffer->GetSurfSize(), startLocation, &m_WindowRect );

		CHAR_INFO* OldChar = m_FlickerFreeBuffer->GetSurfData();
		CHAR_INFO* NewChar = surface.GetSurfData();
		SMALL_RECT WriteRegion;
		COORD OneCharSize = { 1, 1 };

		for( int Y = 0; Y < m_Height; ++Y )
		{
			for( int X = 0; X < m_Width; ++X, ++OldChar, ++NewChar )
			{
				if( OldChar->Char.AsciiChar != NewChar->Char.AsciiChar ||
					OldChar->Attributes != NewChar->Attributes )
				{
					WriteRegion.Left	= WriteRegion.Right		= ( short )X;
					WriteRegion.Top		= WriteRegion.Bottom	= ( short )Y;

					WriteConsoleOutput( m_hConsoleOutput, NewChar, OneCharSize, startLocation, &WriteRegion );
				}
			}
		}
	}
	else
	{
		// Just redraw the whole thing. Somehow, this is slower than the flicker free mode.
		WriteConsoleOutput( m_hConsoleOutput, surface.GetSurfData(), surface.GetSurfSize(), startLocation, &m_WindowRect );
	}
}

void ASCIIRenderer::ShowCursor( bool Show )
{
	ASSERT( m_hConsoleOutput );

	// TODO: Cursor turns back on when I Alt+Enter, so I'll deal with it later.
	// NOTE: Except on Win7, you can't fullscreen a console. Bleh.

	CONSOLE_CURSOR_INFO Cursor;
	GetConsoleCursorInfo( m_hConsoleOutput, &Cursor );

	Cursor.bVisible = Show ? TRUE: FALSE;
	SetConsoleCursorInfo( m_hConsoleOutput, &Cursor );
}

void ASCIIRenderer::SetCursorPosition( int16 x, int16 y )
{
	ASSERT( m_hConsoleOutput );

	COORD c;
	c.X = x;
	c.Y = y;
	SetConsoleCursorPosition( m_hConsoleOutput, c );
}

void ASCIIRenderer::SetFlickerFreeMode( bool FlickerFreeMode )
{
	if( FlickerFreeMode && !m_FlickerFreeBuffer )
	{
		m_FlickerFreeBuffer = new ASCIISurface( m_Width, m_Height );
	}
	else if( !FlickerFreeMode && m_FlickerFreeBuffer )
	{
		SafeDelete( m_FlickerFreeBuffer );
	}
}

bool ASCIIRenderer::GetFlickerFreeMode() const
{
	return m_FlickerFreeBuffer != NULL;
}

/*static*/ uint8 ASCIIRenderer::GetColorFromString( const HashedString& Color )
{
	static const HashedString sBlack( "Black" );
	static const HashedString sBlue( "Blue" );
	static const HashedString sGreen( "Green" );
	static const HashedString sCyan( "Cyan" );
	static const HashedString sRed( "Red" );
	static const HashedString sMagenta( "Magenta" );
	static const HashedString sYellow( "Yellow" );
	static const HashedString sWhite( "White" );
	static const HashedString sBrightBlack( "BrightBlack" );
	static const HashedString sBrightBlue( "BrightBlue" );
	static const HashedString sBrightGreen( "BrightGreen" );
	static const HashedString sBrightCyan( "BrightCyan" );
	static const HashedString sBrightRed( "BrightRed" );
	static const HashedString sBrightMagenta( "BrightMagenta" );
	static const HashedString sBrightYellow( "BrightYellow" );
	static const HashedString sBrightWhite( "BrightWhite" );

	if( Color == sBlack )			{ return FG_BLACK; }
	if( Color == sBlue )			{ return FG_BLUE; }
	if( Color == sGreen )			{ return FG_GREEN; }
	if( Color == sCyan )			{ return FG_CYAN; }
	if( Color == sRed )				{ return FG_RED; }
	if( Color == sMagenta )			{ return FG_MAGENTA; }
	if( Color == sYellow )			{ return FG_YELLOW; }
	if( Color == sWhite )			{ return FG_WHITE; }
	if( Color == sBrightBlack )		{ return FG_BR_BLACK; }
	if( Color == sBrightBlue )		{ return FG_BR_BLUE; }
	if( Color == sBrightGreen )		{ return FG_BR_GREEN; }
	if( Color == sBrightCyan )		{ return FG_BR_CYAN; }
	if( Color == sBrightRed )		{ return FG_BR_RED; }
	if( Color == sBrightMagenta )	{ return FG_BR_MAGENTA; }
	if( Color == sBrightYellow )	{ return FG_BR_YELLOW; }
	if( Color == sBrightWhite )		{ return FG_BR_WHITE; }

	return 0;
}

static const uint sRGBColors[] =
{
	RGB_TO_COLOR( 0x00, 0x00, 0x00 ),
	RGB_TO_COLOR( 0x00, 0x00, 0x80 ),
	RGB_TO_COLOR( 0x00, 0x80, 0x00 ),
	RGB_TO_COLOR( 0x00, 0x80, 0x80 ),
	RGB_TO_COLOR( 0x80, 0x00, 0x00 ),
	RGB_TO_COLOR( 0x80, 0x00, 0x80 ),
	RGB_TO_COLOR( 0x80, 0x80, 0x00 ),
	RGB_TO_COLOR( 0xc0, 0xc0, 0xc0 ),
	RGB_TO_COLOR( 0x80, 0x80, 0x80 ),
	RGB_TO_COLOR( 0x00, 0x00, 0xff ),
	RGB_TO_COLOR( 0x00, 0xff, 0x00 ),
	RGB_TO_COLOR( 0x00, 0xff, 0xff ),
	RGB_TO_COLOR( 0xff, 0x00, 0x00 ),
	RGB_TO_COLOR( 0xff, 0x00, 0xff ),
	RGB_TO_COLOR( 0xff, 0xff, 0x00 ),
	RGB_TO_COLOR( 0xff, 0xff, 0xff )
};

/*static*/ uint ASCIIRenderer::GetRGBColorFromASCIIColor( uint8 Color )
{
	return sRGBColors[ Color & FG_MASK ];
}