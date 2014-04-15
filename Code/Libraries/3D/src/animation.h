#ifndef ANIMATION_H
#define ANIMATION_H

#include "array.h"
#include "hashedstring.h"
#include "vector.h"
#include "angles.h"

#define ANIM_FRAMERATE		30.0f
#define ANIM_NAME_LENGTH	32

class AnimEvent;
class SimpleString;

class Animation
{
public:
	Animation();
	~Animation();

	void				InitializeFromDefinition( const SimpleString& QualifiedAnimationName );

	uint				GetLengthFrames() const;
	float				GetLengthSeconds() const;
	float				GetNonLoopingLengthSeconds() const;

	void				GetVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity ) const;
	Vector				GetDisplacement( float Time ) const;

	char				m_Name[ ANIM_NAME_LENGTH ];	// Only needed for matching entries in animations.config anymore
	HashedString		m_HashedName;				// Used to get animation by name
	uint16				m_StartFrame;				// In bone array's frame list
	uint16				m_Length;					// In frames
	Array< AnimEvent* >	m_AnimEvents;

	// Cheap substitute for root motion
	Vector				m_Velocity;
	Angles				m_RotationalVelocity;
};

#endif // ANIMATION_H