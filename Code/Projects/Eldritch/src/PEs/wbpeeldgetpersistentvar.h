#ifndef WBPEELDGETPERSISTENTVAR_H
#define WBPEELDGETPERSISTENTVAR_H

#include "wbpe.h"

class WBPEEldGetPersistentVar : public WBPE
{
public:
	WBPEEldGetPersistentVar();
	virtual ~WBPEEldGetPersistentVar();

	DEFINE_WBPE_FACTORY( EldGetPersistentVar );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

protected:
	HashedString	m_Key;
};

#endif // WBPEELDGETFACTION_H
