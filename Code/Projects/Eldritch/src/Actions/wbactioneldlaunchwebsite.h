#ifndef WBACTIONELDLAUNCHWEBSITE_H
#define WBACTIONELDLAUNCHWEBSITE_H

#include "wbaction.h"

class WBActionEldLaunchWebSite : public WBAction
{
public:
	WBActionEldLaunchWebSite();
	virtual ~WBActionEldLaunchWebSite();

	DEFINE_WBACTION_FACTORY( EldLaunchWebSite );

	virtual void	Execute();
};

#endif // WBACTIONELDLAUNCHWEBSITE_H