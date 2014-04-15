#ifndef WBPERODINGETLASTKNOWNTIME_H
#define WBPERODINGETLASTKNOWNTIME_H

#include "wbpe.h"

class WBPERodinGetLastKnownTime : public WBPE
{
public:
	WBPERodinGetLastKnownTime();
	virtual ~WBPERodinGetLastKnownTime();

	DEFINE_WBPE_FACTORY( RodinGetLastKnownTime );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*	m_EntityPE;
};

#endif // WBPERODINGETLASTKNOWNTIME_H