#ifndef SPRITE_H
#define SPRITE_H

#include "map.h"
#include "array.h"
#include "hashedstring.h"

class Surface;
class SimpleString;

class Sprite
{
	friend class SpriteInstance;

public:
	Sprite();
	Sprite( Surface* pSurface );
	~Sprite();

	void	UseSurface( Surface* pSurface );

	void	TransformedMaskedBlit( Surface& Dest, int dx, int dy, float Scale, float RotationRadians );

	void	SetColorKey( uint Color );
	void	SetAlpha( float Alpha );

	int		GetWidth();
	int		GetHeight();

	void	InitializeAnimations( const SimpleString& SpriteName );
	bool	IsAnimated() const;

private:
	int			m_Width;	// These will be frame dimensions if animated and surface dimensions if not
	int			m_Height;	// (which amounts to the same thing since non-animated is basically 1 frame)
	Surface*	m_pSurface;
	uint		m_ColorKey;
	float		m_Alpha;	// Constant blending value... eventually incorporate 32-bit surfaces, tho

	// Animation members
	struct Frame
	{
		int x, y;
		float Duration;
	};

	struct Animation
	{
		Array< Frame >	m_Frames;
	};

	Map< HashedString, Animation* >	m_Animations;
};

#endif // SPRITE_H