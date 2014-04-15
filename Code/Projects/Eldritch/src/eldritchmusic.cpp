#include "core.h"
#include "eldritchmusic.h"
#include "isoundinstance.h"
#include "eldritchframework.h"

EldritchMusic::EldritchMusic()
:	m_MusicInstance( NULL )
{
	IAudioSystem* const pAudioSystem = EldritchFramework::GetInstance()->GetAudioSystem();
	ASSERT( pAudioSystem );

	SInstanceDeleteCallback Callback;
	Callback.m_Callback	= InstanceDeleteCallback;
	Callback.m_Void		= this;
	pAudioSystem->RegisterInstanceDeleteCallback( Callback );
}

EldritchMusic::~EldritchMusic()
{
	IAudioSystem* const pAudioSystem = EldritchFramework::GetInstance()->GetAudioSystem();
	ASSERT( pAudioSystem );

	if( m_MusicInstance )
	{
		pAudioSystem->RemoveSoundInstance( m_MusicInstance );
	}

	SInstanceDeleteCallback Callback;
	Callback.m_Callback	= InstanceDeleteCallback;
	Callback.m_Void		= this;
	pAudioSystem->UnregisterInstanceDeleteCallback( Callback );
}

void EldritchMusic::PlayMusic( const SimpleString& MusicSoundDef )
{
	StopMusic();

	if( MusicSoundDef == "" )
	{
		return;
	}

	IAudioSystem* const pAudioSystem = EldritchFramework::GetInstance()->GetAudioSystem();
	ASSERT( pAudioSystem );

	m_MusicInstance = pAudioSystem->CreateSoundInstance( MusicSoundDef );
	ASSERT( m_MusicInstance );

	m_MusicInstance->Tick();
	m_MusicInstance->Play();
}

void EldritchMusic::StopMusic()
{
	if( m_MusicInstance )
	{
		m_MusicInstance->Stop();
	}
}

/*static*/ void EldritchMusic::InstanceDeleteCallback( void* pVoid, ISoundInstance* pInstance )
{
	EldritchMusic* pMusic = static_cast<EldritchMusic*>( pVoid );
	ASSERT( pMusic );

	pMusic->OnInstanceDeleted( pInstance );
}

void EldritchMusic::OnInstanceDeleted( ISoundInstance* const pInstance )
{
	if( pInstance == m_MusicInstance )
	{
		m_MusicInstance = NULL;
	}
}