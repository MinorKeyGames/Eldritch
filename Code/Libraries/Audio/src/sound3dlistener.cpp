#include "core.h"
#include "sound3dlistener.h"
#include "angles.h"

Sound3DListener::Sound3DListener()
:	m_Location()
,	m_Forward()
,	m_Right()
{
}

Sound3DListener::~Sound3DListener()
{
}

void Sound3DListener::SetRotation( const Angles& Rotation )
{
	Vector DummyUp;
	Rotation.GetAxes( m_Right, m_Forward, DummyUp );
}

/*virtual*/ void Sound3DListener::ModifyAttenuation( ISoundInstance* const pSoundInstance, float& Attenuation ) const
{
	Unused( pSoundInstance );
	Unused( Attenuation );
}