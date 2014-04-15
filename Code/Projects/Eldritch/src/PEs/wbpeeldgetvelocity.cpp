#include "core.h"
#include "wbpeeldgetvelocity.h"
#include "Components/wbcompeldtransform.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"

WBPEEldGetVelocity::WBPEEldGetVelocity()
:	m_EntityPE( NULL )
{
}

WBPEEldGetVelocity::~WBPEEldGetVelocity()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEEldGetVelocity::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEEldGetVelocity::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	if( !Context.m_Entity )
	{
		return;
	}

	WBParamEvaluator::SEvaluatedParam Value;
	m_EntityPE->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Entity );

	WBEntity* const pEntity = Value.m_Entity.Get();
	if( !pEntity )
	{
		return;
	}

	WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
	if( !pTransform )
	{
		return;
	}

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Vector;
	EvaluatedParam.m_Vector	= pTransform->GetVelocity();
}