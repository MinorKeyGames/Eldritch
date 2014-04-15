#include "core.h"
#include "wbpemul.h"

WBPEMul::WBPEMul()
{
}

WBPEMul::~WBPEMul()
{
}

void WBPEMul::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam ValueA;
	m_InputA->Evaluate( Context, ValueA );

	WBParamEvaluator::SEvaluatedParam ValueB;
	m_InputB->Evaluate( Context, ValueB );

	if( ValueA.m_Type == WBParamEvaluator::EPT_Vector )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Vector;
		EvaluatedParam.m_Vector = ValueA.m_Vector * ValueB.GetFloat();
	}
	else if( ValueB.m_Type == WBParamEvaluator::EPT_Vector )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Vector;
		EvaluatedParam.m_Vector = ValueB.m_Vector * ValueA.GetFloat();
	}
	else if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
		EvaluatedParam.m_Int = ValueA.m_Int * ValueB.m_Int;
	}
	else
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float = ValueA.GetFloat() * ValueB.GetFloat();
	}
}