#include "core.h"
#include "wbpenot.h"

WBPENOT::WBPENOT()
{
}

WBPENOT::~WBPENOT()
{
}

void WBPENOT::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam Value;
	m_Input->Evaluate( Context, Value );

	EvaluatedParam.m_Type = WBParamEvaluator::EPT_Bool;
	EvaluatedParam.m_Bool = !Value.GetBool();
}