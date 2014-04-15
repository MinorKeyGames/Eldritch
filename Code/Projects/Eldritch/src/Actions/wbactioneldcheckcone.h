#ifndef WBACTIONELDCHECKCONE_H
#define WBACTIONELDCHECKCONE_H

#include "wbaction.h"
#include "wbparamevaluator.h"

class WBActionEldCheckCone : public WBAction
{
public:
	WBActionEldCheckCone();
	virtual ~WBActionEldCheckCone();

	DEFINE_WBACTION_FACTORY( EldCheckCone );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

	virtual void	Execute();

private:
	void			GetConeTransform( Vector& OutLocation, Angles& OutOrientation ) const;

	float			m_ConeCosTheta;
	float			m_ConeLengthSq;
	HashedString	m_CheckTag;
};

#endif // WBACTIONELDCHECKCONE_H