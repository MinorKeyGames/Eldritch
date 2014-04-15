#ifndef ISOUNDINSTANCE_H
#define ISOUNDINSTANCE_H

#include "iaudiosystem.h"

class ISound;
class Sound3DListener;
class View;
class Vector;
class HashedString;

class ISoundInstance
{
public:
	virtual ~ISoundInstance() {}

	virtual void					Play() = 0;
	virtual void					Stop() = 0;
	virtual void					SetPaused( bool Paused ) = 0;
	virtual void					SetVolume( float Volume ) = 0;
	virtual void					SetPan( float Pan ) = 0;
	virtual void					SetLocation( const Vector& Location ) = 0;
	virtual void					SetPriority( ESoundPriority Priority ) const = 0;

	virtual bool					IsPlaying() const = 0;
	virtual bool					IsFinished() const = 0;
	virtual ISound*					GetSound() const = 0;
	virtual float					GetAttenuation() const = 0;
	virtual float					GetTimeElapsed() const = 0;

	virtual void					SetBaseVolume( const float BaseVolume ) = 0;
	virtual float					GetBaseVolume() const = 0;

	virtual Vector					GetLocation() const = 0;

	// Stuff that just gets forwarded up the chain to the sound or audio system
	virtual const HashedString&		GetCategory() const = 0;
	virtual float					GetCategoryVolume() const = 0;
	virtual const Sound3DListener*	Get3DListener() const = 0;
	virtual float					GetMasterVolume() const = 0;
	virtual bool					ShouldCalcOcclusion() const = 0;

	virtual void					Tick() = 0;
	virtual void					Tick3D() = 0;
};

#endif // ISOUNDINSTANCE_H