#include "core.h"
#include "wbpegetname.h"

WBPEGetName::WBPEGetName()
{
}

WBPEGetName::~WBPEGetName()
{
}

void WBPEGetName::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam Value;
	m_Input->Evaluate( Context, Value );

	ASSERT( Value.m_Type == WBParamEvaluator::EPT_Entity );

	if( Value.m_Type != WBParamEvaluator::EPT_Entity )
	{
		return;
	}

	WBEntity* const pEntity = Value.m_Entity.Get();
	if( !pEntity )
	{
		return;
	}

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_String;
	EvaluatedParam.m_String	= pEntity->GetName();
}