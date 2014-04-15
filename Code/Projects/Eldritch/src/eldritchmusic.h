#ifndef ELDRITCHMUSIC_H
#define ELDRITCHMUSIC_H

class ISoundInstance;

class EldritchMusic
{
public:
	EldritchMusic();
	~EldritchMusic();

	void	PlayMusic( const SimpleString& MusicSoundDef );
	void	StopMusic();

	static void		InstanceDeleteCallback( void* pVoid, ISoundInstance* pInstance );

private:
	void			OnInstanceDeleted( ISoundInstance* const pInstance );

	ISoundInstance*	m_MusicInstance;
};

#endif // ELDRITCHMUSIC_H