#include "core.h"
#include "wbpemin.h"
#include "mathcore.h"

WBPEMin::WBPEMin()
{
}

WBPEMin::~WBPEMin()
{
}

void WBPEMin::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
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
		EvaluatedParam.m_Int = Min( ValueA.m_Int, ValueB.m_Int );
	}
	else
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float = Min( ValueA.GetFloat(), ValueB.GetFloat() );
	}
}