#ifndef WBPEGETSTATE_H
#define WBPEGETSTATE_H

#include "wbpe.h"

class WBPEGetState : public WBPE
{
public:
	WBPEGetState();
	virtual ~WBPEGetState();

	DEFINE_WBPE_FACTORY( GetState );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

protected:
	WBPE*	m_EntityPE;
};

#endif // WBPEGETSTATE_H