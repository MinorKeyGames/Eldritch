#ifndef WBACTIONELDTWEETRIP_H
#define WBACTIONELDTWEETRIP_H

#include "wbaction.h"

class WBActionEldTweetRIP : public WBAction
{
public:
	WBActionEldTweetRIP();
	virtual ~WBActionEldTweetRIP();

	DEFINE_WBACTION_FACTORY( EldTweetRIP );

	virtual void	Execute();
};

#endif // WBACTIONELDTWEETRIP_H