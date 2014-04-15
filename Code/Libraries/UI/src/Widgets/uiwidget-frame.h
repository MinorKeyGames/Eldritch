#ifndef UIWIDGETFRAME_H
#define UIWIDGETFRAME_H

#include "uiwidget.h"
#include "vector2.h"
#include "simplestring.h"

class ITexture;
class Mesh;

class UIWidgetFrame : public UIWidget
{
public:
	UIWidgetFrame();
	virtual ~UIWidgetFrame();

	DEFINE_UIWIDGET_FACTORY( Frame );

	virtual void	Render( bool HasFocus );
	virtual void	UpdateRender();
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	GetBounds( SRect& OutBounds );
	virtual float	GetWidth() { return m_Dimensions.x; }
	virtual float	GetHeight() { return m_Dimensions.y; }

private:
	Mesh*			CreateMesh() const;

	ITexture*		m_Texture;
	Vector2			m_Dimensions;
	float			m_Border;
	Mesh*			m_Mesh;
	SimpleString	m_Material;
};

#endif // UIWIDGETFRAME_H