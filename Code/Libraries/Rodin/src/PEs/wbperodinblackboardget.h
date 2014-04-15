#ifndef WBPERODINBLACKBOARDGET_H
#define WBPERODINBLACKBOARDGET_H

#include "wbpe.h"

class WBPERodinBlackboardGet : public WBPE
{
public:
	WBPERodinBlackboardGet();
	virtual ~WBPERodinBlackboardGet();

	DEFINE_WBPE_FACTORY( RodinBlackboardGet );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	HashedString	m_BlackboardKey;	// Config
};

#endif // WBPERODINBLACKBOARDGET_H