#ifndef UIWIDGETSPACER_H
#define UIWIDGETSPACER_H

#include "uiwidget.h"
#include "vector2.h"

class UIWidgetSpacer : public UIWidget
{
public:
	UIWidgetSpacer();
	virtual ~UIWidgetSpacer();

	DEFINE_UIWIDGET_FACTORY( Spacer );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	GetBounds( SRect& OutBounds );
	virtual float	GetWidth() { return m_Dimensions.x; }
	virtual float	GetHeight() { return m_Dimensions.y; }

	Vector2			m_Dimensions;
};

#endif // UIWIDGETSPACER_H