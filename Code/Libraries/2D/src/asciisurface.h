#ifndef ASCIISURFACE_H
#define ASCIISURFACE_H

#include <windows.h>	// For CHAR_INFO and COORD

#define ASURF_FG 1
#define ASURF_BG 2

class ASCIISurface
{
public:
	ASCIISurface( uint w = 80, uint h = 25 );
	~ASCIISurface();

	void Init( uint w = 80, uint h = 25, uint8 ch = ' ', uint8 co = 0 );
	void Fill( uint8 ch, uint8 color );
	void FillRect( uint8 ch, uint8 color, int x, int y, int w, int h );
	void SetChar( uint8 ch, uint8 color, int x, int y, int Flags = 0 );
	void SetChar( uint8 ch, int x, int y );
	void SetColor( uint8 color, int x, int y, int Flags = 0 );
	void SetChar( CHAR_INFO ch, int x, int y, int Flags = 0 );
	void SetColorChar( uint16 ch, int x, int y );

	uint8 GetChar( int x, int y ) const;
	uint8 GetColor( int x, int y ) const;

	void Print(const char * str, uint8 color, int x, int y, int Flags = 0 );	// Very basic print without wrapping
	void PrintCentered( const char* String, uint8 Color, int Y, int XBase = 0, int XSpan = 0, int Flags = 0);
	void Print(const char * str, uint MaxChars, uint8 color, int x, int y, int Flags = 0 );

	void DrawLine( int StartX, int StartY, int EndX, int EndY, uint8 ch, uint8 color, int Flags = 0 );

	CHAR_INFO*	GetSurfData() const { return m_Surface; }
	COORD		GetSurfSize() const { return m_SurfaceSize; }
	uint		GetSurfNumBytes() const { return sizeof( CHAR_INFO ) * m_SurfaceSize.X * m_SurfaceSize.Y; }

	static void	ABlit(const ASCIISurface &src, ASCIISurface &dst, uint16 srcX = 0, uint16 srcY = 0, uint16 srcW = 0, uint16 srcH = 0, int16 dstX = 0, int16 dstY = 0, int Flags = 0, bool masked = false, uint8 maskChar = ' ' );

private:
	bool		CheckBounds( int x, int y ) const;
	CHAR_INFO&	GetCharAt( int x, int y ) const;

	CHAR_INFO*	m_Surface;
	COORD		m_SurfaceSize;
};

#endif	// ASCIISURFACE_H