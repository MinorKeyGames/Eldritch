#ifndef WBCOMPELDSOUND_H
#define WBCOMPELDSOUND_H

#include "wbeldritchcomponent.h"
#include "array.h"

class ISoundInstance;
class SimpleString;
class Vector;

class WBCompEldSound : public WBEldritchComponent
{
public:
	WBCompEldSound();
	virtual ~WBCompEldSound();

	DEFINE_WBCOMP( EldSound, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	static void		InstanceDeleteCallback( void* pVoid, ISoundInstance* pInstance );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			OnInstanceDeleted( ISoundInstance* const pInstance );

	void			StopCategory( const HashedString& Category );
	void			PlaySoundDef( const SimpleString& SoundDef, const Vector& Location, const bool Attached, const float VolumeOverride );

	struct SSoundInstance
	{
		SSoundInstance()
		:	m_SoundInstance( NULL )
		,	m_Attached( false )
		{
		}

		ISoundInstance*	m_SoundInstance;
		bool			m_Attached;
	};

	Array<SSoundInstance>	m_SoundInstances;
};

#endif // WBCOMPELDUSABLE_H