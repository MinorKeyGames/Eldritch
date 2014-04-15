#ifndef SOUND3DLISTENER_H
#define SOUND3DLISTENER_H

#include "vector.h"

class Angles;
class ISoundInstance;

class Sound3DListener
{
public:
	Sound3DListener();
	~Sound3DListener();

	void			SetLocation( const Vector& Location )	{ m_Location = Location; }
	const Vector&	GetLocation() const						{ return m_Location; }

	void			SetRotation( const Angles& Rotation );
	const Vector&	GetForward() const						{ return m_Forward; }
	const Vector&	GetRight() const						{ return m_Right; }

	virtual void	ModifyAttenuation( ISoundInstance* const pSoundInstance, float& Attenuation ) const;

protected:
	Vector	m_Location;
	Vector	m_Forward;
	Vector	m_Right;
};

#endif // SOUND3DLISTENER_H