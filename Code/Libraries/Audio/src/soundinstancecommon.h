#ifndef SOUNDINSTANCECOMMON_H
#define SOUNDINSTANCECOMMON_H

#include "vector.h"
#include "isoundinstance.h"

class ISound;

class SoundInstanceCommon : public ISoundInstance
{
public:
	SoundInstanceCommon();
	virtual ~SoundInstanceCommon();

	virtual void					SetLocation( const Vector& Location ) { m_Location = Location; }

	virtual ISound*					GetSound() const { return m_Sound; }
	virtual float					GetAttenuation() const { return m_Attenuation; }
	virtual void					SetBaseVolume( const float BaseVolume ) { m_BaseVolume = BaseVolume; }
	virtual float					GetBaseVolume() const { return m_BaseVolume; }

	virtual const HashedString&		GetCategory() const;
	virtual float					GetCategoryVolume() const;
	virtual const Sound3DListener*	Get3DListener() const;
	virtual float					GetMasterVolume() const;
	virtual bool					ShouldCalcOcclusion() const;

	virtual void					Tick();
	virtual void					Tick3D();

	virtual Vector					GetLocation() const { return m_Location; }
	void							SetSound( ISound* const pSound ) { m_Sound = pSound; }

protected:
	ISound*	m_Sound;
	float	m_Attenuation;
	float	m_BaseVolume;
	Vector	m_Location;
};

#endif // SOUNDINSTANCECOMMON_H