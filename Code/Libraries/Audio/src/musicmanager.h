#ifndef MUSICMANAGER_H
#define MUSICMANAGER_H

// Queues up and plays pieces of dynamic music based on given moods.
// This "double buffers" the sequences so that the next sequence can
// be started with as little latency as possible; a side effect is that
// mood changes take an extra sequence to apply. I did experiment with
// loading the next sequence just in time, but the latency was worse
// and less consistent, so I'm just going to stick with this.

#include "simplestring.h"

class IAudioSystem;
class ISound;
class ISoundInstance;

class MusicManager
{
public:
	MusicManager( IAudioSystem* pAudioSystem );
	~MusicManager();

	void	PlaySequence( const SimpleString& DefinitionName );
	void	SetMood( const SimpleString& Mood );
	void	Tick();
	void	Flush();

	SimpleString	GetCurrentSequenceName() const;
	SimpleString	GetMood() const;

	static void	InstanceDeleteCallback( void* pVoid, ISoundInstance* pInstance );

private:
	struct SSequence
	{
		SSequence() : m_Sound( NULL ), m_Instance( NULL ), m_DefinitionName( "" ) {}
		ISound*			m_Sound;
		ISoundInstance*	m_Instance;
		SimpleString	m_DefinitionName;
	};

	void	OnInstanceDeleted( ISoundInstance* pInstance );
	void	PlayNextSequence();
	void	QueueNextSequence();
	void	UpdateMusicLevel();
	void	FreeSequence( SSequence& Sequence );	// Stop and null out the sequence

	IAudioSystem*	m_AudioSystem;
	SSequence		m_CurrentSequence;
	SSequence		m_NextSequence;
	SimpleString	m_Mood;

	float			m_NextSequenceLeadTime;
	float			m_NextSequenceQueueTime;
};

#endif // MUSICMANAGER_H