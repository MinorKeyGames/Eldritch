#include "core.h"
#include "spritemanager.h"
#include "sprite.h"
#include "surfacemanager.h"
#include "configmanager.h"

SpriteManager* SpriteManager::m_Instance = NULL;

SpriteManager::SpriteManager()
:	m_Sprites()
{
}

SpriteManager::~SpriteManager()
{
	FOR_EACH_MAP( SpriteIter, m_Sprites, HashedString, Sprite* )
	{
		SafeDelete( *SpriteIter );
	}
	m_Sprites.Clear();
}

SpriteManager* SpriteManager::GetInstance()
{
	if( !m_Instance )
	{
		m_Instance = new SpriteManager;
	}
	return m_Instance;
}

void SpriteManager::DeleteInstance()
{
	SafeDelete( m_Instance );
}

Sprite* SpriteManager::GetSprite( const SimpleString& SpriteDef )
{
	STATICHASH( SurfaceFilename );
	STATICHASH( SurfaceFileType );
	MAKEHASH( SpriteDef );

	SpriteMap::Iterator SpriteIter = m_Sprites.Search( SpriteDef );

	if( SpriteIter.IsNull() )
	{
		Surface* pSurface = SurfaceManager::GetInstance()->GetSurface(
			ConfigManager::GetString( sSurfaceFilename, "", sSpriteDef ),
			( Surface::ESurfaceFileType )ConfigManager::GetInt( sSurfaceFileType, 0, sSpriteDef ) );

		Sprite* pNewSprite = new Sprite( pSurface );
		pNewSprite->InitializeAnimations( SpriteDef );

		SpriteIter = m_Sprites.Insert( SpriteDef, pNewSprite );
	}

	return *SpriteIter;
}