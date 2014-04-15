#ifndef WBACTIONUISHOWYESNODIALOG_H
#define WBACTIONUISHOWYESNODIALOG_H

#include "wbaction.h"
#include "simplestring.h"

class WBActionUIShowYesNoDialog : public WBAction
{
public:
	WBActionUIShowYesNoDialog();
	virtual ~WBActionUIShowYesNoDialog();

	DEFINE_WBACTION_FACTORY( UIShowYesNoDialog );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	SimpleString		m_YesNoString;	// Config
	Array<WBAction*>	m_YesActions;	// Config
};

#endif // WBACTIONUISHOWYESNODIALOG_H