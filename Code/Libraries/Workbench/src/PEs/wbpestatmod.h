#ifndef WBPESTATMOD_H
#define WBPESTATMOD_H

#include "wbpeunaryop.h"
#include "hashedstring.h"

class WBPEStatMod : public WBPEUnaryOp
{
public:
	WBPEStatMod();
	virtual ~WBPEStatMod();

	DEFINE_WBPE_FACTORY( StatMod );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

private:
	HashedString	m_StatName;
	WBPE*			m_EntityPE;
};

#endif // WBPESTATMOD_H
