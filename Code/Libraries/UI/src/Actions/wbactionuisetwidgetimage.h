#ifndef WBACTIONUISETWIDGETIMAGE_H
#define WBACTIONUISETWIDGETIMAGE_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionUISetWidgetImage : public WBAction
{
public:
	WBActionUISetWidgetImage();
	virtual ~WBActionUISetWidgetImage();

	DEFINE_WBACTION_FACTORY( UISetWidgetImage );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	HashedString		m_ScreenName;	// Config
	HashedString		m_WidgetName;	// Config
	HashedString		m_Image;		// Config
	WBParamEvaluator	m_ImagePE;		// Config
};

#endif // WBACTIONUISETWIDGETIMAGE_H