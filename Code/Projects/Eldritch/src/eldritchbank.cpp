#include "core.h"
#include "eldritchbank.h"
#include "wbeventmanager.h"
#include "wbworld.h"
#include "wbentity.h"
#include "Components/wbcompeldplayer.h"
#include "Components/wbcompeldwallet.h"
#include "wbscene.h"
#include "mathcore.h"
#include "configmanager.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "eldritchpersistence.h"

EldritchBank::EldritchBank()
{
	RegisterForEvents();
}

EldritchBank::~EldritchBank()
{
	// I don't unregister for events here because world has already been destroyed. Assumptions!
}

void EldritchBank::RegisterForEvents()
{
	STATIC_HASHED_STRING( OnMasterFileLoaded );
	WBWorld::GetInstance()->GetEventManager()->AddObserver( sOnMasterFileLoaded, this, NULL );
}

/*virtual*/ void EldritchBank::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	STATIC_HASHED_STRING( OnMasterFileLoaded );
	STATIC_HASHED_STRING( BankTransaction );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnMasterFileLoaded )
	{
		PublishToHUD();
	}
	else if( EventName == sBankTransaction )
	{
		STATIC_HASHED_STRING( Amount );
		const int Amount = Event.GetInt( sAmount );

		TryBankTransaction( Amount );
	}
}

// Negative values are withdrawals from the bank to the player.
void EldritchBank::TryBankTransaction( const int Amount )
{
	WBEntity* const pPlayer = EldritchGame::GetPlayer();
	ASSERT( pPlayer );

	WBCompEldWallet* const pWallet = GET_WBCOMP( pPlayer, EldWallet );
	ASSERT( pWallet );

	const int Limit					= pWallet->GetLimit();
	const int CurrentPlayerMoney	= pWallet->GetMoney();
	const int CurrentBankMoney		= GetBankMoney();

	int ClampedAmount = CurrentPlayerMoney - Clamp( CurrentPlayerMoney - Amount, 0, Limit );	// Clamp to player limits
	ClampedAmount = Clamp( CurrentBankMoney + ClampedAmount, 0, Limit ) - CurrentBankMoney;		// Clamp to bank limits

	if( ClampedAmount < 0 )
	{
		// Withdrawal
		const bool ShowPickupScreen = false;
		pWallet->AddMoney( -ClampedAmount, ShowPickupScreen );
		RemoveBankMoney( -ClampedAmount );
	}
	else if( ClampedAmount > 0 )
	{
		// Deposit
		pWallet->RemoveMoney( ClampedAmount );
		AddBankMoney( ClampedAmount );
	}
}

uint EldritchBank::GetBankMoney() const
{
	EldritchPersistence* const pPersistence = EldritchFramework::GetInstance()->GetGame()->GetPersistence();
	return pPersistence->GetBankMoney();
}

void EldritchBank::SetBankMoney( const uint Money )
{
	EldritchPersistence* const pPersistence = EldritchFramework::GetInstance()->GetGame()->GetPersistence();
	return pPersistence->SetBankMoney( Money );
}

void EldritchBank::AddBankMoney( uint Money )
{
	SetBankMoney( GetBankMoney() + Money );

	PublishToHUD();
}

void EldritchBank::RemoveBankMoney( uint Money )
{
	ASSERT( HasBankMoney( Money ) );

	SetBankMoney( GetBankMoney() - Money );

	PublishToHUD();
}

void EldritchBank::PublishToHUD() const
{
	STATICHASH( Bank );
	STATICHASH( BankMoney );
	ConfigManager::SetInt( sBankMoney, GetBankMoney(), sBank );
}