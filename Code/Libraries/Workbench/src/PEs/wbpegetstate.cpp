#include "core.h"
#include "wbpegetstate.h"
#include "Components/wbcompstate.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "reversehash.h"

WBPEGetState::WBPEGetState()
:	m_EntityPE( NULL )
{
}

WBPEGetState::~WBPEGetState()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEGetState::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEGetState::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam EntityValue;
	m_EntityPE->Evaluate( Context, EntityValue );

	WBEntity* const pEntity = EntityValue.GetEntity();
	if( !pEntity )
	{
		return;
	}

	WBCompState* const pState = GET_WBCOMP( pEntity, State );
	if( !pState )
	{
		return;
	}

	const SimpleString State = ReverseHash::ReversedHash( pState->GetState() );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_String;
	EvaluatedParam.m_String	= State;
}