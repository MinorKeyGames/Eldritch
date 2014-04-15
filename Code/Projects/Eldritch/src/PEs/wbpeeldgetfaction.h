#ifndef WBPEELDGETFACTION_H
#define WBPEELDGETFACTION_H

#include "wbpe.h"

class WBPEEldGetFaction : public WBPE
{
public:
	WBPEEldGetFaction();
	virtual ~WBPEEldGetFaction();

	DEFINE_WBPE_FACTORY( EldGetFaction );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

protected:
	WBPE*	m_EntityPE;
};

#endif // WBPEELDGETFACTION_H
