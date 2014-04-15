#ifndef WBACTIONELDPLAYANIM_H
#define WBACTIONELDPLAYANIM_H

#include "wbaction.h"
#include "hashedstring.h"
#include "wbparamevaluator.h"

class WBActionEldPlayAnim : public WBAction
{
public:
	WBActionEldPlayAnim();
	virtual ~WBActionEldPlayAnim();

	DEFINE_WBACTION_FACTORY( EldPlayAnim );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	HashedString		m_AnimationName;
	bool				m_Loop;
	WBParamEvaluator	m_PlayRatePE;
};

#endif // WBACTIONELDPLAYANIM_H