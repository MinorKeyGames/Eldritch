#include "core.h"
#include "wbpeabs.h"
#include "mathcore.h"

WBPEAbs::WBPEAbs()
{
}

WBPEAbs::~WBPEAbs()
{
}

void WBPEAbs::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam Value;
	m_Input->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Int || Value.m_Type == WBParamEvaluator::EPT_Float );

	if( Value.m_Type == WBParamEvaluator::EPT_Int )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
		EvaluatedParam.m_Int = Abs( Value.m_Int );
	}
	else
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float = Abs( Value.GetFloat() );
	}
}