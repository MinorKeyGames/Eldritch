#ifndef FONT_H
#define FONT_H

#include "array.h"
#include "simplestring.h"
#include "map.h"
#include "vector2.h"

#define FONT_PRINT_LEFT		0x00000001
#define FONT_PRINT_CENTER	0x00000002
#define FONT_PRINT_RIGHT	0x00000004

class IRenderer;
class ITexture;
class Mesh;
struct SRect;
class IDataStream;
class AABB;

// Defines a single character's properties in a font. Mapped by code point in SFontLocale.
struct SFontCharProp
{
	SFontCharProp()
	:	m_U1( 0.0f )
	,	m_U2( 0.0f )
	,	m_V1( 0.0f )
	,	m_V2( 0.0f )
	,	m_A( 0 )
	,	m_B( 0 )
	,	m_C( 0 )
	,	m_Width( 0.0f )
	{
	}

	float			m_U1;
	float			m_U2;
	float			m_V1;
	float			m_V2;
	int				m_A;
	unsigned int	m_B;
	int				m_C;
	float			m_Width;	// Size of glyph on sheet including padding (1 + max(0,a) + b + max(0,c))
};

// Defines the font properties and texture sheets for a particular character set (e.g., Latin alphabet).
// These may be shared between languages (e.g., EFIGS may all share one alphabet).
struct SFontLocale
{
	SFontLocale()
	:	m_CharProps()
	,	m_Texture( NULL )
	{
	}

	bool operator==( const SFontLocale& Other ) const
	{
		// Just used for a reverse map lookup for debugging purposes.
		return this == &Other;
	}

	Map<unicode_t, SFontCharProp>	m_CharProps;
	ITexture*						m_Texture;
};

struct STypesetGlyph
{
	STypesetGlyph()
	:	m_CodePoint( 0 )
	,	m_Position()
	{
	}

	unicode_t	m_CodePoint;
	Vector2		m_Position;
};

class Font
{
public:
	Font();
	~Font();

	void		Initialize( const IDataStream& Stream, IRenderer* const pRenderer );
	Mesh*		Print( const SimpleString& StringUTF8, const SRect& Bounds, uint Flags ) const;
	Mesh*		Print( const Array<unicode_t>& CodePoints, const SRect& Bounds, uint Flags ) const;
	void		Arrange( const SimpleString& StringUTF8, const SRect& Bounds, Array<STypesetGlyph>& OutTypesetting, Vector2& OutDimensions ) const;
	void		Arrange( const Array<unicode_t>& CodePoints, const SRect& Bounds, Array<STypesetGlyph>& OutTypesetting, Vector2& OutDimensions ) const;

private:
	enum EWordResult
	{
		EWR_None,		// Used to signal the end of the string
		EWR_Word,
		EWR_Linebreak,
	};

	const SFontLocale&		GetCurrentLocale() const;
	const SFontCharProp&	GetCharProp( const SFontLocale& Locale, const unicode_t CodePoint ) const;
	EWordResult				GetNextWord( const Array<unicode_t>& CodePoints, const SFontLocale& Locale, uint& Index, Array<unicode_t>& WordBuffer, float& WordWidth ) const;

	IRenderer*						m_Renderer;
	float							m_Height;
	Map<HashedString, SFontLocale>	m_FontLocales;		// Map from locale tag to properties (e.g. hash("Latin") -> Latin character set)
	Map<HashedString, HashedString>	m_FontLanguages;	// Map from language to locale tag (e.g. hash("English") -> hash("Latin"))
};

#endif // FONT_H