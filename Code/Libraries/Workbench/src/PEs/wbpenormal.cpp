#include "core.h"
#include "wbpenormal.h"

WBPENormal::WBPENormal()
{
}

WBPENormal::~WBPENormal()
{
}

void WBPENormal::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam Value;
	m_Input->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Vector );

	if( Value.m_Type == WBParamEvaluator::EPT_Vector )
	{
		EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Vector;
		EvaluatedParam.m_Vector	= Value.m_Vector.GetNormalized();
	}
}