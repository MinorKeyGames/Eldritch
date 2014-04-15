#include "core.h"
#include "spriteinstance.h"
#include "surface.h"

SpriteInstance::SpriteInstance( Sprite* pSprite )
:	m_Sprite( pSprite )
,	m_CurrentAnimation( NULL )
,	m_CurrentFrame( 0 )
,	m_FrameTime( 0.0f )
,	m_Looping( false )
{
}

SpriteInstance::~SpriteInstance()
{
}

void SpriteInstance::PlayAnimation( const HashedString& AnimationName, bool Looping )
{
	ASSERT( m_Sprite );

	m_CurrentAnimation = m_Sprite->m_Animations[ AnimationName ];
	m_CurrentFrame = 0;
	m_FrameTime = 0.0f;
	m_Looping = Looping;
}

void SpriteInstance::Tick( float DeltaTime )
{
	if( m_CurrentAnimation )
	{
		int NumFrames = m_CurrentAnimation->m_Frames.Size();
		bool CanAdvance = m_Looping || ( m_CurrentFrame + 1 < NumFrames );
		float FrameDuration = m_CurrentAnimation->m_Frames[ m_CurrentFrame ].Duration;

		m_FrameTime += DeltaTime;

		while( CanAdvance && m_FrameTime > FrameDuration )
		{
			m_FrameTime -= FrameDuration;
			m_CurrentFrame = ( m_CurrentFrame + 1 ) % NumFrames;

			CanAdvance = m_Looping || ( m_CurrentFrame + 1 < NumFrames );
			FrameDuration = m_CurrentAnimation->m_Frames[ m_CurrentFrame ].Duration;
		}
	}
}

void SpriteInstance::BlitToSurface( Surface& Dest, int dx, int dy, uint Flags )
{
	ASSERT( m_Sprite );

	if( m_Sprite->IsAnimated() && m_CurrentAnimation )
	{
		Sprite::Frame& Frame = m_CurrentAnimation->m_Frames[ m_CurrentFrame ];
		Surface::Blit( *m_Sprite->m_pSurface, Dest, dx, dy, m_Sprite->m_Width, m_Sprite->m_Height, Frame.x, Frame.y, Flags, m_Sprite->m_ColorKey, m_Sprite->m_Alpha );
	}
	else
	{
		Surface::Blit( *m_Sprite->m_pSurface, Dest, dx, dy, m_Sprite->m_Width, m_Sprite->m_Height, 0, 0, Flags, m_Sprite->m_ColorKey, m_Sprite->m_Alpha );
	}
}