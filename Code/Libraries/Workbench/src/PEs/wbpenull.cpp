#include "core.h"
#include "wbpenull.h"
#include "configmanager.h"

WBPENull::WBPENull()
{
}

WBPENull::~WBPENull()
{
}

/*virtual*/ void WBPENull::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
	EvaluatedParam.m_Entity	= NULL;
}