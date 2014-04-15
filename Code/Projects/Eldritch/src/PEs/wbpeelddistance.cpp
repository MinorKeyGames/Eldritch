#include "core.h"
#include "wbpeelddistance.h"
#include "Components/wbcompeldtransform.h"

WBPEEldDistance::WBPEEldDistance()
{
}

WBPEEldDistance::~WBPEEldDistance()
{
}

/*virtual*/ void WBPEEldDistance::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam ValueA;
	m_InputA->Evaluate( Context, ValueA );

	WBParamEvaluator::SEvaluatedParam ValueB;
	m_InputB->Evaluate( Context, ValueB );

	ASSERT( ValueA.m_Type == WBParamEvaluator::EPT_Entity );
	ASSERT( ValueB.m_Type == WBParamEvaluator::EPT_Entity );

	if( ValueA.m_Type != WBParamEvaluator::EPT_Entity || ValueB.m_Type != WBParamEvaluator::EPT_Entity )
	{
		return;
	}

	WBEntity* const pEntityA = ValueA.m_Entity.Get();
	WBEntity* const pEntityB = ValueB.m_Entity.Get();

	if( !pEntityA || !pEntityB )
	{
		return;
	}

	WBCompEldTransform* const pTransformA = pEntityA->GetTransformComponent<WBCompEldTransform>();
	WBCompEldTransform* const pTransformB = pEntityB->GetTransformComponent<WBCompEldTransform>();

	ASSERT( pTransformA );
	ASSERT( pTransformB );

	const float Distance = ( pTransformB->GetLocation() - pTransformA->GetLocation() ).Length();

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Float;
	EvaluatedParam.m_Float	= Distance;
}