#include "core.h"
#include "3d.h"
#include "fontmanager.h"
#include "irenderer.h"
#include "font.h"
#include "packstream.h"

#include <string.h>

FontManager::FontManager( IRenderer* Renderer ) : m_Renderer( Renderer ) {}

FontManager::~FontManager()
{
	FreeFonts();
}

void FontManager::FreeFonts()
{
	FOR_EACH_MAP( Iter, m_FontTable, HashedString, Font* )
	{
		SafeDelete( *Iter );
	}
	m_FontTable.Clear();
}

Font* FontManager::GetFont( const char* Filename )
{
	HashedString HashedFilename( Filename );

	Map<HashedString, Font*>::Iterator FontIter = m_FontTable.Search( HashedFilename );
	if( FontIter.IsValid() )
	{
		return FontIter.GetValue();
	}
	else
	{
		Font* const pFont = new Font;
		pFont->Initialize( PackStream( Filename ), m_Renderer );

		m_FontTable.Insert( HashedFilename, pFont );

		return pFont;
	}
}