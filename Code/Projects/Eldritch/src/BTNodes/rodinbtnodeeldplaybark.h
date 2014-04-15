#ifndef RODINBTNODEELDPLAYBARK_H
#define RODINBTNODEELDPLAYBARK_H

#include "rodinbtnode.h"
#include "iwbeventobserver.h"
#include "wbparamevaluator.h"

class RodinBTNodeEldPlayBark : public RodinBTNode
{
public:
	RodinBTNodeEldPlayBark();
	virtual ~RodinBTNodeEldPlayBark();

	DEFINE_RODINBTNODE_FACTORY( EldPlayBark );

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual ETickStatus	Tick( float DeltaTime );

private:
	SimpleString		m_SoundDef;
	WBParamEvaluator	m_SoundDefPE;
	HashedString		m_Category;
};

#endif // RODINBTNODEELDPLAYBARK_H