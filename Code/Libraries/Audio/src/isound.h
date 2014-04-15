#ifndef ISOUND_H
#define ISOUND_H

#include "iaudiosystem.h"	// For ESoundPriority

class ISoundInstance;
class Sound3DListener;
class HashedString;

class ISound
{
public:
	virtual ~ISound() {}

	virtual ISoundInstance*			CreateSoundInstance() = 0;

	virtual IAudioSystem*			GetAudioSystem() const = 0;
	virtual bool					GetIsStream() const = 0;
	virtual bool					GetIs3D() const = 0;
	virtual float					GetLength() const = 0;
	virtual float					GetBaseVolume() const = 0;
	virtual float					GetFalloffDistance() const = 0;
	virtual float					GetMinimumAttenuation() const = 0;
	virtual float					GetBiasedPan( const float Distance ) const = 0;
	virtual float					GetPanPower() const = 0;
	virtual float					GetRearAttenuation() const = 0;
	virtual const HashedString&		GetCategory() const = 0;
	virtual ESoundPriority			GetPriority() const = 0;

	// Stuff that just gets forwarded up to audio system
	virtual float					GetCategoryVolume() const = 0;
	virtual const Sound3DListener*	Get3DListener() const = 0;
	virtual float					GetMasterVolume() const = 0;
	virtual bool					ShouldCalcOcclusion() const = 0;
};

#endif // ISOUND_H