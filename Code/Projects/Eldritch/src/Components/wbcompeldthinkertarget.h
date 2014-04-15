#ifndef WBCOMPELDTHINKERTARGET_H
#define WBCOMPELDTHINKERTARGET_H

#include "wbcompeldthinker.h"

class WBCompEldThinkerTarget : public WBCompEldThinker
{
public:
	WBCompEldThinkerTarget();
	virtual ~WBCompEldThinkerTarget();

	DEFINE_WBCOMP( EldThinkerTarget, WBCompEldThinker );

	virtual void	Tick( float DeltaTime );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_OutputCombatTargetBlackboardKey;
	HashedString	m_OutputSearchTargetBlackboardKey;

	float			m_CombatTargetScoreThreshold;
	float			m_SearchTargetScoreThreshold;
};

#endif // WBCOMPELDTHINKERTARGET_H