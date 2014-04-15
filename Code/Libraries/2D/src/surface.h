#ifndef SURFACE_H
#define SURFACE_H

#include "bmp-format.h"

class Window;

// Wraps a Windows DIB (or equivalent on other platforms

#if BUILD_WINDOWS_NO_SDL
#include <Windows.h>
#endif

#if BUILD_SDL
#include "SDL2/SDL.h"
#endif

class IDataStream;

#define BLIT_MASK		0x1
#define BLIT_ALPHA		0x2	// In this context, alpha does not imply an alpha channel
#define BLIT_BLEND		0x4	// Blend and alpha are the same except blend is always 50/50 so it's faster
#define BLIT_ADDITIVE	0x8

#define DEFAULT_COLOR_KEY	0x00ff00ff	// Magenta

class Surface
{
public:
	enum ESurfaceFileType
	{
		ESFT_BMP,
		ESFT_TGA,
	};

	Surface();
	Surface( int Width, int Height );
	Surface( const IDataStream& Stream, ESurfaceFileType FileType );
	~Surface();

	void Reset( int Width, int Height, int BitCount );
	void BlitToWindow( Window* const pWindow ) const;	// Assumes that the bitmap is the same size as the window
	void SaveBMP( const IDataStream& Stream );

	void Clear( unsigned int Color );

	void DrawPoint( int x, int y, uint Color );
	void DrawLine( int x1, int y1, int x2, int y2, uint Color );
	void DrawBox( int x1, int y1, int x2, int y2, uint Color );
	void DrawFilledBox( int x1, int y1, int x2, int y2, uint Color );

	// Grab raw data
	byte* GetPointerAt( int x, int y );
	const byte* GetPointerAt( int x, int y ) const;
	uint GetColorAtPoint( int x, int y ) const;

	int GetWidth() const;
	int GetHeight() const;
	int GetPadding() const;
	int GetStride() const;

#if BUILD_WINDOWS_NO_SDL
	HDC	GetHDC();
#endif
#if BUILD_SDL
	SDL_Surface* GetSDLSurface() const { return m_Surface; }
#endif

	// Viewport is used to clip rectangles
	void SetViewport( int Left, int Top, int Right, int Bottom );

	// All-purpose blit
	static void Blit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint Flags, uint ColorKey = DEFAULT_COLOR_KEY, float Alpha = 0.5f );

	// Returns true if dimensions are valid after clipping
	static bool ClipRectangles( const Surface& Src, const Surface& Dest, int& DstX, int& DstY, int& W, int& H, int& SrcX, int& SrcY );

	// Special case (for pixel art): scale one surface up to another by integer value
	static void ScaledBlit( const Surface& Src, Surface& Dest, int Scalar );

	static void StandardBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy );
	static void MaskBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint ColorKey );
	static void MaskBlendBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint ColorKey );
	static void MaskAlphaBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, uint ColorKey, float Alpha );
	static void BlendBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy );
	static void AlphaBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy, float Alpha );
	static void AdditiveBlit( const Surface& Src, Surface& Dest, int dx, int dy, int w, int h, int sx, int sy );
	static void TransformedMaskedBlit( const Surface& Src, Surface& Dest, int dx, int dy, float Scale, float RotationRadians, uint ColorKey );

private:
	int ComputeStride( const int Width, const int BitCount );
	int ComputePadding();

	int LoadBMP( const IDataStream& Stream );
	int LoadTGA( const IDataStream& Stream );

	int				m_Width;
	int				m_Height;
	int				m_Stride;
	int				m_Padding;
#if BUILD_WINDOWS_NO_SDL
	HDC				m_hDC;
	HBITMAP			m_BMP;
	HBITMAP			m_OldBMP;
#endif
#if BUILD_SDL
	SDL_Surface*	m_Surface;
#endif
	unsigned char*	m_pPixels;
	SBitmapInfo		m_BitmapInfo;
	int				m_ViewportLeft;
	int				m_ViewportTop;
	int				m_ViewportRight;
	int				m_ViewportBottom;
};

#endif // SURFACE_H