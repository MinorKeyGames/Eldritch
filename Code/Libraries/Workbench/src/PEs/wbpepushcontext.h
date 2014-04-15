#ifndef WBPEPUSHCONTEXT_H
#define WBPEPUSHCONTEXT_H

// This is essentially a decorator for PEs that switches context by pushing a
// new event onto the action stack and creating a new context for PE evaluation.

#include "wbpeunaryop.h"

class WBPEPushContext : public WBPEUnaryOp
{
public:
	WBPEPushContext();
	virtual ~WBPEPushContext();

	DEFINE_WBPE_FACTORY( PushContext );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*	m_EntityPE;
};

#endif // WBPEPUSHCONTEXT_H
