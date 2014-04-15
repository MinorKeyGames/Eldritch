#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "map.h"
#include "hashedstring.h"

#define DEFAULT_TEXTURE "Textures/default.dds"
#define DEFAULT_NORMAL "Textures/default_NORMAL.dds"
#define DEFAULT_SPEC "Textures/default_SPEC.dds"

class IRenderer;
class ITexture;

class TextureManager
{
public:
	TextureManager( IRenderer* Renderer );
	~TextureManager();

	// Should be sorted from least to most permanent
	enum ETextureLife
	{
		ETL_Null,
		ETL_World,		// For environments and characters, only persist for a level
		ETL_Permanent,	// For fonts, UI, etc.
	};

	struct SManagedTexture
	{
		SManagedTexture() : m_Texture( NULL ), m_Life( ETL_Null ) {}
		ITexture*		m_Texture;
		ETextureLife	m_Life;
	};

	void		FreeTextures( ETextureLife Life );
	ITexture*	GetTexture( const char* Filename, ETextureLife Life = ETL_Permanent );

private:
	Map<HashedString, SManagedTexture>	m_TextureTable;
	IRenderer*							m_Renderer;
};

#endif // TEXTUREMANAGER_H