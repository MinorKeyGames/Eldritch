#ifndef DISPLAY_H
#define DISPLAY_H

#include "array.h"

struct SDisplayMode
{
	SDisplayMode()
	:	Width( 0 )
	,	Height( 0 )
	{
	}

	uint	Width;
	uint	Height;
	// Could also add bit depth and refresh rate here, but I'm
	// just going to assume that the config values work.

	bool operator==( const SDisplayMode& Mode ) const
	{
		return
			Width == Mode.Width &&
			Height == Mode.Height;
	}

	// Sort by width first.
	bool operator<( const SDisplayMode& Mode ) const
	{
		if( Width < Mode.Width )
		{
			return true;
		}

		if( Width > Mode.Width )
		{
			return false;
		}

		return Height < Mode.Height;
	}
};

class Display
{
public:
	Display();
	~Display();

	// Side effects: updates config vars to keep in sync with display
	void	SetFullScreen( bool FullScreen );
	void	SetResolution( uint Width, uint Height );
	void	UpdateDisplay();	// Call SetDisplay using current members
	bool	NeedsUpdate();

	static void			EnumerateDisplayModes( Array<SDisplayMode>& DisplayModes );
	static SDisplayMode	GetBestDisplayMode( const uint DesiredWidth, const uint DesiredHeight );
	static void			SetDisplay( bool Reset = true, bool FullScreen = false, int Width = 0, int Height = 0, int BitDepth = 0, int Frequency = 0 );

	uint	m_Width;
	uint	m_Height;
	uint	m_ScreenWidth;	// Initial Windows display resolution (as opposed to game's resolution)
	uint	m_ScreenHeight;	// Not updated when user changes resolutions, so it's always the stored default.
	float	m_AspectRatio;
	bool	m_Fullscreen;
	Array< SDisplayMode > m_DisplayModes;
};

#endif // DISPLAY_H