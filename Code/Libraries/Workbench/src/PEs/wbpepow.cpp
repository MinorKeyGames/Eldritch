#include "core.h"
#include "wbpepow.h"
#include "mathcore.h"

WBPEPow::WBPEPow()
{
}

WBPEPow::~WBPEPow()
{
}

void WBPEPow::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam ValueA;
	m_InputA->Evaluate( Context, ValueA );

	WBParamEvaluator::SEvaluatedParam ValueB;
	m_InputB->Evaluate( Context, ValueB );

	ASSERT( ValueA.m_Type == WBParamEvaluator::EPT_Int || ValueA.m_Type == WBParamEvaluator::EPT_Float );
	ASSERT( ValueB.m_Type == WBParamEvaluator::EPT_Int || ValueB.m_Type == WBParamEvaluator::EPT_Float );

	if( ValueA.m_Type == WBParamEvaluator::EPT_Int && ValueB.m_Type == WBParamEvaluator::EPT_Int )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
		EvaluatedParam.m_Int = ( int )Pow( ( float )ValueA.m_Int, ( float )ValueB.m_Int );
	}
	else
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float = Pow( ValueA.GetFloat(), ValueB.GetFloat() );
	}
}