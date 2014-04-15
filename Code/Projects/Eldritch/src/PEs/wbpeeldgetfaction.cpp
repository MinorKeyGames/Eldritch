#include "core.h"
#include "wbpeeldgetfaction.h"
#include "Components/wbcompeldfaction.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "reversehash.h"

WBPEEldGetFaction::WBPEEldGetFaction()
:	m_EntityPE( NULL )
{
}

WBPEEldGetFaction::~WBPEEldGetFaction()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEEldGetFaction::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEEldGetFaction::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam EntityValue;
	m_EntityPE->Evaluate( Context, EntityValue );

	WBEntity* const pEntity = EntityValue.GetEntity();
	if( !pEntity )
	{
		return;
	}

	WBCompEldFaction* const pFaction = GET_WBCOMP( pEntity, EldFaction );
	if( !pFaction )
	{
		return;
	}

	const SimpleString Faction = ReverseHash::ReversedHash( pFaction->GetFaction() );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_String;
	EvaluatedParam.m_String	= Faction;
}