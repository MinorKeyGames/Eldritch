#ifndef WBACTIONELDPLAYHANDANIM_H
#define WBACTIONELDPLAYHANDANIM_H

#include "wbaction.h"
#include "hashedstring.h"

class WBActionEldPlayHandAnim : public WBAction
{
public:
	WBActionEldPlayHandAnim();
	virtual ~WBActionEldPlayHandAnim();

	DEFINE_WBACTION_FACTORY( EldPlayHandAnim );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	HashedString	m_AnimationName;
};

#endif // WBACTIONELDPLAYANIM_H