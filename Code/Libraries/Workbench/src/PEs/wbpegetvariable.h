#ifndef WBPEGETVARIABLE_H
#define WBPEGETVARIABLE_H

#include "wbpe.h"

class WBPEGetVariable : public WBPE
{
public:
	WBPEGetVariable();
	virtual ~WBPEGetVariable();

	DEFINE_WBPE_FACTORY( GetVariable );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	WBPE*			m_EntityPE;
	HashedString	m_VariableName;
};

#endif // WBPEGETVARIABLE_H