#include "core.h"
#include "wbpeand.h"

WBPEAND::WBPEAND()
{
}

WBPEAND::~WBPEAND()
{
}

void WBPEAND::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam ValueA;
	m_InputA->Evaluate( Context, ValueA );

	WBParamEvaluator::SEvaluatedParam ValueB;
	m_InputB->Evaluate( Context, ValueB );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Bool;
	EvaluatedParam.m_Bool = ValueA.GetBool() && ValueB.GetBool();
}