#ifndef WBACTIONUISHOWHIDEWIDGET_H
#define WBACTIONUISHOWHIDEWIDGET_H

#include "wbaction.h"

class WBActionUIShowHideWidget : public WBAction
{
public:
	WBActionUIShowHideWidget();
	virtual ~WBActionUIShowHideWidget();

	DEFINE_WBACTION_FACTORY( UIShowHideWidget );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	HashedString	m_ScreenName;	// Config
	HashedString	m_WidgetName;	// Config
	bool			m_Hidden;		// Config
	bool			m_SetDisabled;	// Config: also set disabled if hiding, or set enabled if showing
};

#endif // WBACTIONUISHOWHIDEWIDGET_H