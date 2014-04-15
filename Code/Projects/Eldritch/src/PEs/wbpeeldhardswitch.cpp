#include "core.h"
#include "wbpeeldhardswitch.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "eldritchgame.h"
#include "Components/wbcompeldhard.h"

WBPEEldHardSwitch::WBPEEldHardSwitch()
:	m_NormalInput( NULL )
,	m_HardInput( NULL )
{
}

WBPEEldHardSwitch::~WBPEEldHardSwitch()
{
	SafeDelete( m_NormalInput );
	SafeDelete( m_HardInput );
}

void WBPEEldHardSwitch::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	STATICHASH( Normal );
	STATICHASH( Hard );
	MAKEHASH( DefinitionName );

	m_NormalInput = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sNormal, "", sDefinitionName ) );
	m_HardInput = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sHard, "", sDefinitionName ) );

	ASSERT( m_NormalInput );
	ASSERT( m_HardInput );
}

void WBPEEldHardSwitch::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBEntity* const pPlayer = EldritchGame::GetPlayer();
	ASSERT( pPlayer );

	WBCompEldHard* const pHard = SAFE_GET_WBCOMP( pPlayer, EldHard );
	ASSERT( pHard );

	if( pHard && pHard->IsHard() )
	{
		m_HardInput->Evaluate( Context, EvaluatedParam );
	}
	else
	{
		m_NormalInput->Evaluate( Context, EvaluatedParam );
	}
}