#include "core.h"
#include "wbpestatmod.h"
#include "configmanager.h"
#include "Components/wbcompstatmod.h"
#include "wbparamevaluatorfactory.h"

WBPEStatMod::WBPEStatMod()
:	m_StatName()
,	m_EntityPE( NULL )
{
}

WBPEStatMod::~WBPEStatMod()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEStatMod::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBPEUnaryOp::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( StatName );
	m_StatName = ConfigManager::GetHash( sStatName, HashedString::NullString, sDefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEStatMod::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	ASSERT( Context.m_Entity );

	WBParamEvaluator::SEvaluatedParam EntityValue;
	m_EntityPE->Evaluate( Context, EntityValue );

	WBEntity* const			pEntity		= EntityValue.GetEntity();
	WBCompStatMod* const	pStatMod	= SAFE_GET_WBCOMP( pEntity, StatMod );

	WBParamEvaluator::SEvaluatedParam Value;
	m_Input->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Int || Value.m_Type == WBParamEvaluator::EPT_Float );

	if( Value.m_Type == WBParamEvaluator::EPT_Int )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
		EvaluatedParam.m_Int = pStatMod ? static_cast<int>( pStatMod->ModifyFloat( Value.GetFloat(), m_StatName ) ) : Value.m_Int;
	}
	else
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float = pStatMod ? pStatMod->ModifyFloat( Value.GetFloat(), m_StatName ) : Value.GetFloat();
	}
}