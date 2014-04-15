#ifndef WBPEELDGETITEM_H
#define WBPEELDGETITEM_H

#include "wbpe.h"

class WBPEEldGetItem : public WBPE
{
public:
	WBPEEldGetItem();
	virtual ~WBPEEldGetItem();

	DEFINE_WBPE_FACTORY( EldGetItem );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const;

protected:
	WBPE*	m_EntityPE;
	WBPE*	m_SlotPE;
};

#endif // WBPEELDGETITEM_H
