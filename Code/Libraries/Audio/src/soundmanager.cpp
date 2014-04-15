#include "core.h"
#include "soundmanager.h"
#include "iaudiosystem.h"
#include "isound.h"

SoundManager::SoundManager( IAudioSystem* pAudioSystem )
:	m_AudioSystem( pAudioSystem )
{
}

SoundManager::~SoundManager()
{
	FreeSounds();
}

void SoundManager::FreeSounds()
{
	// NOTE: Streaming sounds are not automatically freed; they
	// must be freed by the caller.
	FOR_EACH_MAP( Iter, m_SoundTable, HashedString, ISound* )
	{
		SafeDelete( Iter.GetValue() );
	}
	m_SoundTable.Clear();
}

ISound* SoundManager::GetSound( const SSoundInit& SoundInit, const SimpleString& SoundDefinitionName )
{
	if( SoundInit.m_IsStream )
	{
		// NOTE: Streaming sounds are not added to the map, because requesting
		// the same stream twice needs to create two separate instances. For this
		// reason, the calling code needs to handle the pointer.
		ISound* const pSound = m_AudioSystem->CreateSound( SoundInit );
		ASSERT( pSound );

		return pSound;
	}
	else
	{
		const HashedString HashedSoundDef = SoundDefinitionName;

		const TSoundMap::Iterator SoundIter = m_SoundTable.Search( HashedSoundDef );
		if( SoundIter.IsNull() )
		{
			ISound* const pSound = m_AudioSystem->CreateSound( SoundInit );
			ASSERT( pSound );

			m_SoundTable.Insert( HashedSoundDef, pSound );

			return pSound;
		}
		else
		{
			return SoundIter.GetValue();
		}
	}
}