#include "core.h"
#include "wbpesquare.h"
#include "mathcore.h"

WBPESquare::WBPESquare()
{
}

WBPESquare::~WBPESquare()
{
}

void WBPESquare::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam Value;
	m_Input->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Int || Value.m_Type == WBParamEvaluator::EPT_Float );

	if( Value.m_Type == WBParamEvaluator::EPT_Int )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Int;
		EvaluatedParam.m_Int = Square( Value.m_Int );
	}
	else
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Float;
		EvaluatedParam.m_Float = Square( Value.GetFloat() );
	}
}