#ifndef WBACTIONELDGIVEITEM_H
#define WBACTIONELDGIVEITEM_H

#include "wbaction.h"
#include "simplestring.h"
#include "wbparamevaluator.h"

class WBActionEldGiveItem : public WBAction
{
public:
	WBActionEldGiveItem();
	virtual ~WBActionEldGiveItem();

	DEFINE_WBACTION_FACTORY( EldGiveItem );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	void			GiveItemTo( const SimpleString& ItemDef,  WBEntity* const pEntity ) const;

	SimpleString		m_ItemDef;
	WBParamEvaluator	m_ItemDefPE;
	WBParamEvaluator	m_GiveToPE;
};

#endif // WBACTIONELDGIVEITEM_H