#ifndef WBACTIONUIPUSHSCREEN_H
#define WBACTIONUIPUSHSCREEN_H

#include "wbaction.h"

class WBActionUIPushScreen : public WBAction
{
public:
	WBActionUIPushScreen();
	virtual ~WBActionUIPushScreen();

	DEFINE_WBACTION_FACTORY( UIPushScreen );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Execute();

private:
	HashedString	m_ScreenName;	// Config
};

#endif // WBACTIONUIPUSHSCREEN_H