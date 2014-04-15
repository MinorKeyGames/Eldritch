#ifndef WBCOMPELDSENSORHEARING_H
#define WBCOMPELDSENSORHEARING_H

#include "wbcompeldsensor.h"

class Vector;

class WBCompEldSensorHearing : public WBCompEldSensor
{
public:
	WBCompEldSensorHearing();
	virtual ~WBCompEldSensorHearing();

	DEFINE_WBCOMP( EldSensorHearing, WBCompEldSensor );

	virtual void	HandleEvent( const WBEvent& Event );

	bool	GetNoiseCertainty( const Vector& NoiseLocation, const float NoiseRadius, float& OutCertainty ) const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void	HandleNoise( WBEntity* const pNoiseEntity, const Vector& NoiseLocation, const float NoiseRadius, const float NoiseUpdateTime, const float ExpireTimeBonus ) const;

	float	m_Radius;					// Config; adds to noise's radius, so some AIs can hear further than others. Can be negative.
	float	m_CertaintyFalloffRadius;	// Config
	float	m_DistanceCertaintyFactor;	// Config
	float	m_OcclusionCertaintyFactor;	// Config
};

#endif // WBCOMPELDSENSORVISION_H