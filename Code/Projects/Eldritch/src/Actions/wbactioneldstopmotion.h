#ifndef WBACTIONELDSTOPMOTION_H
#define WBACTIONELDSTOPMOTION_H

#include "wbaction.h"

class WBActionEldStopMotion : public WBAction
{
public:
	WBActionEldStopMotion();
	virtual ~WBActionEldStopMotion();

	DEFINE_WBACTION_FACTORY( EldStopMotion );

	virtual void	Execute();
};

#endif // WBACTIONELDSTOPMOTION_H