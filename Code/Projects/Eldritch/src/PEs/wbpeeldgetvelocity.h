#ifndef WBPEELDGETVELOCITY_H
#define WBPEELDGETVELOCITY_H

#include "wbpe.h"

class WBPEEldGetVelocity : public WBPE
{
public:
	WBPEEldGetVelocity();
	virtual ~WBPEEldGetVelocity();

	DEFINE_WBPE_FACTORY( EldGetVelocity );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*	m_EntityPE;
};

#endif // WBPEELDGETVELOCITY_H
