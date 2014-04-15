#ifndef SPRITEMANAGER_H
#define SPRITEMANAGER_H

#include "map.h"

class Sprite;

class SpriteManager
{
public:
	static SpriteManager* GetInstance();
	static void DeleteInstance();

	Sprite* GetSprite( const SimpleString& SpriteDef );

private:
	SpriteManager();
	~SpriteManager();

	static SpriteManager*	m_Instance;

	typedef Map< HashedString, Sprite* > SpriteMap;
	SpriteMap	m_Sprites;
};

#endif // SPRITEMANAGER_H