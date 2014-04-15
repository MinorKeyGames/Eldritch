#ifndef ASCIIRENDERER_H
#define ASCIIRENDERER_H

#include <windows.h>	// For COORD

#define FG_BLACK		0x00
#define FG_BLUE			0x01
#define FG_GREEN		0x02
#define FG_CYAN			0x03
#define FG_RED			0x04
#define FG_MAGENTA		0x05
#define FG_YELLOW		0x06
#define FG_WHITE		0x07
#define FG_BR_BLACK		0x08
#define FG_BR_BLUE		0x09
#define FG_BR_GREEN		0x0A
#define FG_BR_CYAN		0x0B
#define FG_BR_RED		0x0C
#define FG_BR_MAGENTA	0x0D
#define FG_BR_YELLOW	0x0E
#define FG_BR_WHITE		0x0F
#define BG_BLACK		0x00
#define BG_BLUE			0x10
#define BG_GREEN		0x20
#define BG_CYAN			0x30
#define BG_RED			0x40
#define BG_MAGENTA		0x50
#define BG_YELLOW		0x60
#define BG_WHITE		0x70
#define BG_BR_BLACK		0x80
#define BG_BR_BLUE		0x90
#define BG_BR_GREEN		0xA0
#define BG_BR_CYAN		0xB0
#define BG_BR_RED		0xC0
#define BG_BR_MAGENTA	0xD0
#define BG_BR_YELLOW	0xE0
#define BG_BR_WHITE		0xF0

#define FG_MASK			0x0F
#define BG_MASK			0xF0

#define MAKE_COLOR_CHAR( ch, co ) ( ( ( ch ) << 8 ) | ( co ) )
#define MAKE_BG_COLOR( co ) ( ( co ) << 4 )
#define MAKE_FG_COLOR( co ) ( ( co ) >> 4 )

// Tag these with 1s and 0s to create the desired color (in case math is hard or something, I dunno)
uint8 GetBitWiseColor( uint8 fore_r, uint8 fore_g, uint8 fore_b, uint8 fore_i, uint8 back_r, uint8 back_g, uint8 back_b, uint8 back_i );

class ASCIISurface;

class ASCIIRenderer
{
public:
	ASCIIRenderer();
	~ASCIIRenderer();

	void Init( uint width = 80, uint height = 25 );
	void Draw( const ASCIISurface& surface );

	void ShowCursor( bool Show );
	void SetCursorPosition( int16 x, int16 y );

	void	SetFlickerFreeMode( bool FlickerFreeMode );
	bool	GetFlickerFreeMode() const;

	static uint8	GetColorFromString( const HashedString& Color );
	static uint		GetRGBColorFromASCIIColor( uint8 Color );

private:
	HANDLE			m_hConsoleOutput;
	SMALL_RECT		m_WindowRect;
	ASCIISurface*	m_FlickerFreeBuffer;
	int				m_Width;
	int				m_Height;
};

#endif	// ASCIIRENDERER_H