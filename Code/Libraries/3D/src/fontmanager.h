#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "map.h"
#include "hashedstring.h"

#define DEFAULT_FONT "Fonts/arial8.fnp"

class IRenderer;
class Font;

class FontManager
{
public:
	FontManager( IRenderer* Renderer );
	~FontManager();

	void	FreeFonts();
	Font*	GetFont( const char* Filename );

private:
	Map<HashedString, Font*>	m_FontTable;
	IRenderer*					m_Renderer;
};

#endif // FONTMANAGER_H