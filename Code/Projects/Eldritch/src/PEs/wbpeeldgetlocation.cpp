#include "core.h"
#include "wbpeeldgetlocation.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "Components/wbcompeldtransform.h"

WBPEEldGetLocation::WBPEEldGetLocation()
:	m_EntityPE( NULL )
{
}

WBPEEldGetLocation::~WBPEEldGetLocation()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEEldGetLocation::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Entity );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntity, "", sDefinitionName ) );
}

/*virtual*/ void WBPEEldGetLocation::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam Value;
	m_EntityPE->Evaluate( Context, Value );

	WBEntity* const pEntity = Value.GetEntity();
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
	EvaluatedParam.m_Vector	= pTransform->GetLocation();
}