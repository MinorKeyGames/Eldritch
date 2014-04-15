#ifndef SPRITEINSTANCE_H
#define SPRITEINSTANCE_H

#include "sprite.h"

class SpriteInstance
{
public:
	SpriteInstance( Sprite* pSprite );
	~SpriteInstance();

	// This does what you want 99% of the time: blits the proper frame, or the whole surface if not animated
	void	BlitToSurface( Surface& Dest, int dx, int dy, uint Flags );
	void	PlayAnimation( const HashedString& AnimationName, bool Looping );
	void	Tick( float DeltaTime );

private:
	Sprite*	m_Sprite;

	// Animation state
	Sprite::Animation*	m_CurrentAnimation;
	int					m_CurrentFrame;
	float				m_FrameTime;
	bool				m_Looping;
};

#endif // SPRITEINSTANCE_H