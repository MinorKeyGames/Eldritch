#include "core.h"
#include "font.h"
#include "mesh.h"
#include "irenderer.h"
#include "texturemanager.h"
#include "vector2.h"
#include "ivertexdeclaration.h"
#include "idatastream.h"
#include "mathcore.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "configmanager.h"
#include "reversehash.h"

#include <stdio.h>
#include <string.h>

Font::Font()
:	m_Renderer( NULL )
,	m_Height( 0.0f )
,	m_FontLocales()
,	m_FontLanguages()
{
}

Font::~Font()
{
}

void Font::Initialize( const IDataStream& Stream, IRenderer* const pRenderer )
{
	ASSERT( pRenderer );
	m_Renderer = pRenderer;
	TextureManager* const pTextureManager = pRenderer->GetTextureManager();

	// Add 1 for the half pixel buffer space on either side.
	m_Height = static_cast<float>( Stream.ReadInt32() );

	const uint NumLocales = Stream.ReadUInt32();
	FOR_EACH_INDEX( LocaleIndex, NumLocales )
	{
		const HashedString LocaleTag = Stream.ReadHashedString();
		DEBUGASSERT( m_FontLocales.Search( LocaleTag ).IsNull() );
		SFontLocale& Locale = m_FontLocales.Insert( LocaleTag );

		const SimpleString ImageFilename = Stream.ReadString();
		Locale.m_Texture = pTextureManager->GetTexture( ImageFilename.CStr() );

		const uint NumLanguages = Stream.ReadUInt32();
		FOR_EACH_INDEX( LanguageIndex, NumLanguages )
		{
			const HashedString Language = Stream.ReadHashedString();
			DEBUGASSERT( m_FontLanguages.Search( Language ).IsNull() );
			m_FontLanguages.Insert( Language, LocaleTag );
		}

		const uint NumCodePoints = Stream.ReadUInt32();
		FOR_EACH_INDEX( CodePointIndex, NumCodePoints )
		{
			const unicode_t CodePoint = Stream.ReadUInt32();
			DEBUGASSERT( Locale.m_CharProps.Search( CodePoint ).IsNull() );
			SFontCharProp& CharProp = Locale.m_CharProps.Insert( CodePoint );

			Stream.Read( sizeof( SFontCharProp ), &CharProp );
		}
	}
}

const SFontCharProp& Font::GetCharProp( const SFontLocale& Locale, const unicode_t CodePoint ) const
{
	const Map<unicode_t, SFontCharProp>::Iterator PropIter = Locale.m_CharProps.Search( CodePoint );

	if( PropIter.IsNull() )
	{
#if BUILD_DEV
		Map<HashedString, SFontLocale>::Iterator LocaleIter = m_FontLocales.SearchValue( Locale );
		ASSERT( LocaleIter.IsValid() );
		const HashedString LocaleHash		= LocaleIter.GetKey();
		const SimpleString LocaleString		= ReverseHash::ReversedHash( LocaleHash );
		const SimpleString CodePointString	= SimpleString::GetCodePointString( CodePoint );
		PRINTF( "Code point %s not found in locale %s.\n", CodePointString.CStr(), LocaleString.CStr() );
#endif

		// Return a ?
		return GetCharProp( Locale, 0x003F );
	}

	DEVASSERT( PropIter.IsValid() );
	return PropIter.GetValue();
}

const SFontLocale& Font::GetCurrentLocale() const
{
	const HashedString Language = ConfigManager::GetLanguageHash();

	Map<HashedString, HashedString>::Iterator LanguageIter = m_FontLanguages.Search( Language );
	ASSERT( LanguageIter.IsValid() );
	const HashedString Locale = LanguageIter.GetValue();

	Map<HashedString, SFontLocale>::Iterator LocaleIter = m_FontLocales.Search( Locale );
	ASSERT( LocaleIter.IsValid() );
	return LocaleIter.GetValue();
}

Font::EWordResult Font::GetNextWord( const Array<unicode_t>& CodePoints, const SFontLocale& Locale, uint& Index, Array<unicode_t>& WordBuffer, float& WordWidth ) const
{
	WordBuffer.Clear();
	WordWidth = 0.0f;

#define ADDCODEPOINT												\
	WordBuffer.PushBack( CodePoint );								\
	WordWidth += ( CharProp.m_A + CharProp.m_B + CharProp.m_C );	\
	++Index

	const uint NumCodePoints = CodePoints.Size();
	for( ; Index < NumCodePoints; )
	{
		const unicode_t CodePoint = CodePoints[ Index ];

		if( CodePoint == '\n' )
		{
			if( WordBuffer.Empty() )
			{
				++Index;
				return EWR_Linebreak;
			}
			else
			{
				return EWR_Word;
			}
		}
		else if( CodePoint == ' ' || CodePoint == '\t' )	// Just treat Tab as a whitespace character
		{
			if( WordBuffer.Empty() )
			{
				const SFontCharProp& CharProp = GetCharProp( Locale, CodePoint );
				ADDCODEPOINT;
			}
			return EWR_Word;
		}
		else if( CodePoint == '-' )
		{
			// Add this breaking character to the current word, then return.
			const SFontCharProp& CharProp = GetCharProp( Locale, CodePoint );
			ADDCODEPOINT;
			return EWR_Word;
		}
		else
		{
			const SFontCharProp& CharProp = GetCharProp( Locale, CodePoint );
			ADDCODEPOINT;
		}
	}

#undef ADDCODEPOINT

	return WordBuffer.Empty() ? EWR_None : EWR_Word;
}

// TODO LOC LATER: Add support for center/right alignment. This could be done by adjusting
// typeset positions after each line is closed (or as a post process after all typesetting).
// I wouldn't be too worried about performance on that, since I almost never use it. (Using
// a center or right origin on the UI widget is sufficient for single-line elements.)
void Font::Arrange( const Array<unicode_t>& CodePoints, const SRect& Bounds, Array<STypesetGlyph>& OutTypesetting, Vector2& OutDimensions ) const
{
	// We know we can't exceed the number of code points, at least.
	OutTypesetting.Reserve( CodePoints.Size() );

	// Cache the locale so we don't do multiple lookups while typesetting.
	const SFontLocale& Locale = GetCurrentLocale();

	// Initialize typeset cursor; type will later be adjusted so (0,0)
	// is relative to ( Bounds.m_Left - 1.0f, Bounds.m_Top - 1.0f ).
	Vector2 Cursor;

	const float	BoundWidth	= Bounds.m_Right - Bounds.m_Left;
	const bool	Bounded		= BoundWidth > 0.0f;

	// Initialize temporary word buffer
	Array<unicode_t> WordBuffer;
	WordBuffer.SetDeflate( false );
	float	WordWidth	= 0.0f;
	uint	Index		= 0;

	EWordResult WordResult = EWR_None;
	while( EWR_None != ( WordResult = GetNextWord( CodePoints, Locale, Index, WordBuffer, WordWidth ) ) )
	{
		if( WordResult == EWR_Linebreak )
		{
			// Advance cursor to new line but don't draw anything
			Cursor.x = 0.0f;
			Cursor.y += m_Height;
		}
		else
		{
			ASSERT( WordResult == EWR_Word );

			// Advance cursor to new line if word won't fit.
			if( Bounded &&							// Are we doing a bounded print?
				Cursor.x > 0.0f &&					// Is this not the first word on the line?
				Cursor.x + WordWidth > BoundWidth )	// Does the word not fit within bounds?
			{
				Cursor.x = 0.0f;
				Cursor.y += m_Height;

				// Special case: If the word was a single space, drop it.
				// We don't want a leading space on the line in this case.
				if( WordBuffer.Size() == 1 && WordBuffer[0] == ' ' )
				{
					continue;
				}
			}

			// Add chars to typesetting array and advance cursor
			FOR_EACH_ARRAY( WordIter, WordBuffer, unicode_t )
			{
				const unicode_t			CodePoint	= WordIter.GetValue();
				const SFontCharProp&	CharProp	= GetCharProp( Locale, CodePoint );

				Cursor.x += Min( 0, CharProp.m_A );	// Retreat cursor if there is leading overhang

				OutDimensions.x = Max( OutDimensions.x, Cursor.x + CharProp.m_Width );
				OutDimensions.y = Max( OutDimensions.y, Cursor.y + m_Height + 1.0f );

				STypesetGlyph& TypesetGlyph	= OutTypesetting.PushBack();
				TypesetGlyph.m_CodePoint	= CodePoint;
				TypesetGlyph.m_Position.x	= Bounds.m_Left + Cursor.x - 1.0f;	// Subtract 1/2 for the half pixel padding in the font
				TypesetGlyph.m_Position.y	= Bounds.m_Top + Cursor.y - 1.0f;	// and 1/2 to align on pixel grid

				Cursor.x += Max( 0, CharProp.m_A );	// Advance cursor if there is leading padding
				Cursor.x += CharProp.m_B;
				Cursor.x += CharProp.m_C;
			}
		}
	}
}

Mesh* Font::Print( const SimpleString& StringUTF8, const SRect& Bounds, uint Flags ) const
{
	Array<unicode_t> CodePoints;
	StringUTF8.UTF8ToUnicode( CodePoints );
	return Print( CodePoints, Bounds, Flags );
}

Mesh* Font::Print( const Array<unicode_t>& CodePoints, const SRect& Bounds, uint Flags ) const
{
	// TODO LOC LATER: Support alignment again.
	Unused( Flags );

	// Cache the locale so we don't do multiple lookups while typesetting.
	const SFontLocale& Locale = GetCurrentLocale();

	Array<STypesetGlyph> TypesetGlyphs;
	Vector2 Dimensions;
	Arrange( CodePoints, Bounds, TypesetGlyphs, Dimensions );

	const uint NumVertices	= TypesetGlyphs.Size() * 4;
	const uint NumIndices	= TypesetGlyphs.Size() * 6;

	Array<Vector> Positions;
	Positions.Reserve( NumVertices );

	Array<Vector2> UVs;
	UVs.Reserve( NumVertices );

	Array<index_t> Indices;
	Indices.Reserve( NumIndices );

	FOR_EACH_ARRAY( GlyphIter, TypesetGlyphs, STypesetGlyph )
	{
		const STypesetGlyph& Glyph = GlyphIter.GetValue();

		const SFontCharProp& CharProp = GetCharProp( Locale, Glyph.m_CodePoint );

		Positions.PushBack( Vector( Glyph.m_Position.x,						0.0f, Glyph.m_Position.y + m_Height + 1.0f ) );
		Positions.PushBack( Vector( Glyph.m_Position.x + CharProp.m_Width,	0.0f, Glyph.m_Position.y + m_Height + 1.0f ) );
		Positions.PushBack( Vector( Glyph.m_Position.x,						0.0f, Glyph.m_Position.y ) );
		Positions.PushBack( Vector( Glyph.m_Position.x + CharProp.m_Width,	0.0f, Glyph.m_Position.y ) );

		UVs.PushBack( Vector2( CharProp.m_U1, CharProp.m_V2 ) );
		UVs.PushBack( Vector2( CharProp.m_U2, CharProp.m_V2 ) );
		UVs.PushBack( Vector2( CharProp.m_U1, CharProp.m_V1 ) );
		UVs.PushBack( Vector2( CharProp.m_U2, CharProp.m_V1 ) );

		// Ordered:
		// 2 3
		// 0 1
		const index_t BaseIndex = static_cast<index_t>( Positions.Size() - 4 );
		Indices.PushBack( BaseIndex + 0 );
		Indices.PushBack( BaseIndex + 1 );
		Indices.PushBack( BaseIndex + 2 );
		Indices.PushBack( BaseIndex + 1 );
		Indices.PushBack( BaseIndex + 3 );
		Indices.PushBack( BaseIndex + 2 );
	}

	IVertexBuffer*		pVertexBuffer		= m_Renderer->CreateVertexBuffer();
	IVertexDeclaration*	pVertexDeclaration	= m_Renderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS );
	IIndexBuffer*		pIndexBuffer		= m_Renderer->CreateIndexBuffer();
	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= NumVertices;
	InitStruct.Positions	= Positions.GetData();
	InitStruct.UVs			= UVs.GetData();
	pVertexBuffer->Init( InitStruct );
	pIndexBuffer->Init( NumIndices, Indices.GetData() );
	pIndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

	Mesh* StringMesh		= new Mesh( pVertexBuffer, pVertexDeclaration, pIndexBuffer );

	AABB MeshBounds;
	MeshBounds.m_Min.x = Bounds.m_Left - 1.0f;
	MeshBounds.m_Min.z = Bounds.m_Top - 1.0f;
	MeshBounds.m_Max.x = MeshBounds.m_Min.x + Dimensions.x;
	MeshBounds.m_Max.z = MeshBounds.m_Min.z + Dimensions.y;

	StringMesh->SetTexture( 0, Locale.m_Texture );
	StringMesh->m_AABB		= MeshBounds;
#if BUILD_DEBUG
	StringMesh->m_Name		= "String";
#endif

	return StringMesh;
}