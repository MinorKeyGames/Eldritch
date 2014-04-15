#include "core.h"
#include "sprite.h"
#include "surface.h"

#include "vector.h"
#include "mathcore.h"

#include "configmanager.h"
#include "simplestring.h"

#include <Windows.h>

#define BYTES 3

Sprite::Sprite()
:	m_Width( 0 )
,	m_Height( 0 )
,	m_pSurface( NULL )
,	m_ColorKey( DEFAULT_COLOR_KEY )
,	m_Alpha( 0.5f )
,	m_Animations()
{
}

Sprite::Sprite( Surface* pSurface )
:	m_Width( 0 )
,	m_Height( 0 )
,	m_pSurface( NULL )
,	m_ColorKey( DEFAULT_COLOR_KEY )
,	m_Alpha( 0.5f )
,	m_Animations()
{
	UseSurface( pSurface );
}

Sprite::~Sprite()
{
	FOR_EACH_MAP( AnimIter, m_Animations, HashedString, Sprite::Animation* )
	{
		delete *AnimIter;
	}
	m_Animations.Clear();
}

void Sprite::UseSurface( Surface* pSurface )
{
	m_pSurface = pSurface;

	m_Width = pSurface->GetWidth();
	m_Height = pSurface->GetHeight();
}

// Not deprecated yet, mainly because I'm not using it anywhere
// for reals and don't know what a better interface would be.
void Sprite::TransformedMaskedBlit( Surface& Dest, int dx, int dy, float Scale, float RotationRadians )
{
	Surface::TransformedMaskedBlit( *m_pSurface, Dest, dx, dy, Scale, RotationRadians, m_ColorKey );
}

void Sprite::SetColorKey( uint Color )
{
	m_ColorKey = Color;
}

void Sprite::SetAlpha( float Alpha )
{
	ASSERT( Alpha >= 0.0f && Alpha <= 1.0f );
	m_Alpha = Alpha;
}

int Sprite::GetWidth()
{
	return m_Width;
}

int Sprite::GetHeight()
{
	return m_Height;
}

void Sprite::InitializeAnimations( const SimpleString& SpriteName )
{
	STATICHASH( Width );
	STATICHASH( Height );
	STATICHASH( NumAnimations );
	MAKEHASH( SpriteName );

	m_Width = ConfigManager::GetInt( sWidth, m_pSurface->GetWidth(), sSpriteName );
	m_Height = ConfigManager::GetInt( sHeight, m_pSurface->GetHeight(), sSpriteName );

	m_Animations.Clear();

	ASSERT( m_pSurface );
	ASSERT( m_Width <= m_pSurface->GetWidth() );
	ASSERT( m_Height <= m_pSurface->GetHeight() );

	int FrameColumns = m_pSurface->GetWidth() / m_Width;

	int NumAnimations = ConfigManager::GetInt( sNumAnimations, 0, sSpriteName );
	for( int AnimIndex = 0; AnimIndex < NumAnimations; ++AnimIndex )
	{
		Sprite::Animation* Animation = new Sprite::Animation;
		SimpleString AnimationName = ConfigManager::GetSequenceString( "Animation%d", AnimIndex, "", sSpriteName );

		STATICHASH( NumFrames );
		MAKEHASH( AnimationName );

		int NumFrames = ConfigManager::GetInt( sNumFrames, 0, sAnimationName );
		for( int FrameIndex = 0; FrameIndex < NumFrames; ++FrameIndex )
		{
			Sprite::Frame Frame;

			int FrameNum = ConfigManager::GetSequenceInt( "Frame%d", FrameIndex, 0, sAnimationName );
			Frame.Duration = ConfigManager::GetSequenceFloat( "Duration%d", FrameIndex, 0.0f, sAnimationName );
			Frame.x = ( FrameNum % FrameColumns ) * m_Width;
			Frame.y = ( FrameNum / FrameColumns ) * m_Height;

			ASSERT( Frame.Duration > 0.0f || NumFrames == 1 );

			Animation->m_Frames.PushBack( Frame );
		}

		m_Animations.Insert( HashedString( AnimationName ), Animation );
	}
}

bool Sprite::IsAnimated() const
{
	return m_Animations.Size() > 0;
}