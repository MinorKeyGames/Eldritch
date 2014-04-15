#ifndef UIWIDGETIMAGE_H
#define UIWIDGETIMAGE_H

#include "uiwidget.h"
#include "vector2.h"
#include "simplestring.h"

class ITexture;
class Mesh;

class UIWidgetImage : public UIWidget
{
public:
	UIWidgetImage();
	UIWidgetImage( const SimpleString& DefinitionName );
	virtual ~UIWidgetImage();

	DEFINE_UIWIDGET_FACTORY( Image );

	virtual void	Render( bool HasFocus );
	virtual void	UpdateRender();
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	GetBounds( SRect& OutBounds );
	virtual float	GetWidth() { return m_Dimensions.x; }
	virtual float	GetHeight() { return m_Dimensions.y; }

	void			SetTexture( const char* Filename );
	void			SetTexture( ITexture* const pTexture );

	ITexture*		m_Texture;
	Vector2			m_Dimensions;
	Mesh*			m_Mesh;
	bool			m_Calibration;	// TODO: Replace with shader override
	SimpleString	m_Material;
};

#endif // UIWIDGETIMAGE_H