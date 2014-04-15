#ifndef ELDRITCHSOUND3DLISTENER_H
#define ELDRITCHSOUND3DLISTENER_H

#include "sound3dlistener.h"

class EldritchWorld;

class EldritchSound3DListener : public Sound3DListener
{
public:
	EldritchSound3DListener();
	~EldritchSound3DListener();

	virtual void	ModifyAttenuation( ISoundInstance* const pSoundInstance, float& Attenuation ) const;

	void			Initialize();
	void			SetWorld( EldritchWorld* const pWorld ) { m_World = pWorld; }

private:
	EldritchWorld*	m_World;
	float			m_VerticalScalar;
	float			m_OccludedFalloffRadius;
};

#endif // ELDRITCHSOUND3DLISTENER_H