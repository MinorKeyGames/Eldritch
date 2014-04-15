#include "core.h"
#include "wbpeself.h"
#include "wbevent.h"

WBPESelf::WBPESelf()
{
}

WBPESelf::~WBPESelf()
{
}

/*virtual*/ void WBPESelf::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	if( Context.m_Entity )
	{
		EvaluatedParam.m_Type = WBParamEvaluator::EPT_Entity;
		EvaluatedParam.m_Entity = Context.m_Entity;
	}
}