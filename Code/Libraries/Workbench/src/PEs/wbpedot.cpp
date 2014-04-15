#include "core.h"
#include "wbpedot.h"

WBPEDot::WBPEDot()
{
}

WBPEDot::~WBPEDot()
{
}

void WBPEDot::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam ValueA;
	m_InputA->Evaluate( Context, ValueA );

	WBParamEvaluator::SEvaluatedParam ValueB;
	m_InputB->Evaluate( Context, ValueB );

	ASSERT( ValueA.m_Type == WBParamEvaluator::EPT_Vector && ValueB.m_Type == WBParamEvaluator::EPT_Vector );

	if( ValueA.m_Type == WBParamEvaluator::EPT_Vector && ValueB.m_Type == WBParamEvaluator::EPT_Vector )
	{
		EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float	= ValueA.m_Vector.Dot( ValueB.m_Vector );
	}
}