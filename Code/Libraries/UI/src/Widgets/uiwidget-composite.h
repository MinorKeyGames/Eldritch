#ifndef UIWIDGETCOMPOSITE_H
#define UIWIDGETCOMPOSITE_H

#include "uiwidget.h"
#include "array.h"

class UIWidgetComposite : public UIWidget
{
public:
	UIWidgetComposite();
	virtual ~UIWidgetComposite();

	DEFINE_UIWIDGET_FACTORY( Composite );

	virtual void	Render( bool HasFocus );
	virtual void	UpdateRender();
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	GetBounds( SRect& OutBounds );
	virtual void	Refresh();

protected:
	void			ClearChildren();

	Array<UIWidget*>	m_Children;
};

#endif // UIWIDGETCOMPOSITE_H