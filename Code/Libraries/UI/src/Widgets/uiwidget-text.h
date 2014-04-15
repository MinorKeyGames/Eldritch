#ifndef UIWIDGETTEXT_H
#define UIWIDGETTEXT_H

#include "uiwidget.h"
#include "vector2.h"
#include "simplestring.h"

class Font;
class Mesh;

class UIWidgetText : public UIWidget
{
public:
	UIWidgetText();
	UIWidgetText( const SimpleString& DefinitionName );
	virtual ~UIWidgetText();

	DEFINE_UIWIDGET_FACTORY( Text );

	virtual void	Render( bool HasFocus );
	virtual void	UpdateRender();
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	GetBounds( SRect& OutBounds );
	virtual float	GetWidth();
	virtual float	GetHeight();
	virtual void	Refresh();
	virtual void	Tick( const float DeltaTime );

	void			UpdatePosition();
	void			UpdateRenderPosition();
	void			GetFontPrintFlags( const SimpleString& Alignment );

	const SimpleString&	GetString();		// Returns the best choice from m_String and m_DynamicString

	Font*			m_Font;
	bool			m_IsLiteral;			// Won't look up in string table; only use for numbers!!
	bool			m_IsDynamicPosition;	// Reupdate the position every tick for dynamic strings
	bool			m_ClampToPixelGrid;
	SimpleString	m_String;
	SimpleString	m_DynamicString;
	Mesh*			m_Mesh;
	uint			m_FontPrintFlags;
	float			m_WrapWidth;

	bool			m_HasDropShadow;
	Vector2			m_DropShadowOffset;
	Vector4			m_DropShadowColor;
	Mesh*			m_DropShadowMesh;
};

#endif // UIWIDGETTEXT_H