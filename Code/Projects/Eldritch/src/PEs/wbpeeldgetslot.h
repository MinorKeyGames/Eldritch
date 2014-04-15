#ifndef WBPEELDGETSLOT_H
#define WBPEELDGETSLOT_H

#include "wbpe.h"

class WBPEEldGetSlot : public WBPE
{
public:
	WBPEEldGetSlot();
	virtual ~WBPEEldGetSlot();

	DEFINE_WBPE_FACTORY( EldGetSlot );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

protected:
	WBPE*	m_EntityPE;
};

#endif // WBPEELDGETSLOT_H
