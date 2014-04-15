#include "core.h"
#include "wbactioneldbanktransaction.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "eldritchbank.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBActionEldBankTransaction::WBActionEldBankTransaction()
:	m_Amount( 0 )
{
}

WBActionEldBankTransaction::~WBActionEldBankTransaction()
{
}

/*virtual*/ void WBActionEldBankTransaction::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Amount );
	m_Amount = ConfigManager::GetInt( sAmount, 0, sDefinitionName );
}

/*virtual*/ void WBActionEldBankTransaction::Execute()
{
	WBAction::Execute();

	EldritchGame* const			pGame			= EldritchFramework::GetInstance()->GetGame();
	ASSERT( pGame );

	EldritchBank* const			pBank			= pGame->GetBank();
	ASSERT( pBank );

	WBEventManager* const		pEventManager	= WBWorld::GetInstance()->GetEventManager();
	ASSERT( pEventManager );

	WB_MAKE_EVENT( BankTransaction, NULL );
	WB_LOG_EVENT( BankTransaction );
	WB_SET_AUTO( BankTransaction, Int, Amount, m_Amount );
	WB_DISPATCH_EVENT( pEventManager, BankTransaction, pBank );
}