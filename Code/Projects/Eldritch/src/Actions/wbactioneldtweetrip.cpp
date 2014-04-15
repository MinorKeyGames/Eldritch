#include "core.h"
#include "wbactioneldtweetrip.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "wbeventmanager.h"

WBActionEldTweetRIP::WBActionEldTweetRIP()
{
}

WBActionEldTweetRIP::~WBActionEldTweetRIP()
{
}

/*virtual*/ void WBActionEldTweetRIP::Execute()
{
	WBAction::Execute();

	EldritchGame* const		pGame			= EldritchFramework::GetInstance()->GetGame();
	ASSERT( pGame );

	WBEventManager* const	pEventManager	= WBWorld::GetInstance()->GetEventManager();
	ASSERT( pEventManager );

	WB_MAKE_EVENT( TweetRIP, NULL );
	WB_LOG_EVENT( TweetRIP );
	WB_DISPATCH_EVENT( pEventManager, TweetRIP, pGame );
}