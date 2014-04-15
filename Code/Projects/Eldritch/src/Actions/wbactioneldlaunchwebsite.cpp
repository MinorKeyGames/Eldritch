#include "core.h"
#include "wbactioneldlaunchwebsite.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "wbeventmanager.h"

WBActionEldLaunchWebSite::WBActionEldLaunchWebSite()
{
}

WBActionEldLaunchWebSite::~WBActionEldLaunchWebSite()
{
}

/*virtual*/ void WBActionEldLaunchWebSite::Execute()
{
	WBAction::Execute();

	EldritchGame* const		pGame			= EldritchFramework::GetInstance()->GetGame();
	ASSERT( pGame );

	WBEventManager* const	pEventManager	= WBWorld::GetInstance()->GetEventManager();
	ASSERT( pEventManager );

	WB_MAKE_EVENT( LaunchWebSite, NULL );
	WB_LOG_EVENT( LaunchWebSite );
	WB_DISPATCH_EVENT( pEventManager, LaunchWebSite, pGame );
}