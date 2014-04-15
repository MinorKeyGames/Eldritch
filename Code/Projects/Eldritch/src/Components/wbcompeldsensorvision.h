#ifndef WBCOMPELDSENSORVISION_H
#define WBCOMPELDSENSORVISION_H

#include "wbcompeldsensorpoll.h"

class WBCompEldSensorVision : public WBCompEldSensorPoll
{
public:
	WBCompEldSensorVision();
	virtual ~WBCompEldSensorVision();

	DEFINE_WBCOMP( EldSensorVision, WBCompEldSensorPoll );

#if BUILD_DEV
	virtual void	DebugRender() const;
#endif

	bool			IsVisible( WBEntity* const pVisibleEntity ) const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	PollTick( const float DeltaTime ) const;

private:
	float	m_EyeOffsetZ;				// Config
	float	m_RadiusSq;					// Config
	float	m_ConeCos;					// Config
	float	m_ConeInvZScale;			// Config
	float	m_CertaintyFalloffRadius;	// Config
	float	m_DistanceCertaintyFactor;	// Config
	float	m_CertaintyVelocity;		// Config
	float	m_CertaintyDecay;			// Config
};

#endif // WBCOMPELDSENSORVISION_H