#ifndef WBPEQUERYACTIONSTACK_H
#define WBPEQUERYACTIONSTACK_H

#include "wbpe.h"

class WBPEQueryActionStack : public WBPE
{
public:
	WBPEQueryActionStack();
	virtual ~WBPEQueryActionStack();

	DEFINE_WBPE_FACTORY( QueryActionStack );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	HashedString	m_Key;
};

#endif // WBPECONSTANTFLOAT_H
