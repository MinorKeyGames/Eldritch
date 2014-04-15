#ifndef WBPERODINGETLASTKNOWNLOCATION_H
#define WBPERODINGETLASTKNOWNLOCATION_H

#include "wbpe.h"

class WBPERodinGetLastKnownLocation : public WBPE
{
public:
	WBPERodinGetLastKnownLocation();
	virtual ~WBPERodinGetLastKnownLocation();

	DEFINE_WBPE_FACTORY( RodinGetLastKnownLocation );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*	m_EntityPE;
};

#endif // WBPERODINGETLASTKNOWNLOCATION_H