#ifndef WBCOMPELDTHINKERHEARWORMTARGET_H
#define WBCOMPELDTHINKERHEARWORMTARGET_H

#include "wbcompeldthinker.h"

class WBCompEldThinkerHearWormTarget : public WBCompEldThinker
{
public:
	WBCompEldThinkerHearWormTarget();
	virtual ~WBCompEldThinkerHearWormTarget();

	DEFINE_WBCOMP( EldThinkerHearWormTarget, WBCompEldThinker );

	virtual void	Tick( float DeltaTime );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_OutputAlarmTargetBlackboardKey;
	HashedString	m_OutputWatchTargetBlackboardKey;

	float			m_AlarmTargetScoreThreshold;
	float			m_WatchTargetScoreThreshold;
};

#endif // WBCOMPELDTHINKERHEARWORMTARGET_H