#ifndef ELDRITCHGAME_H
#define ELDRITCHGAME_H

#include "iwbeventobserver.h"
#include "wbevent.h"
#include "vector2.h"

class EldritchSaveLoad;
class EldritchPersistence;
class EldritchBank;
class EldritchMusic;
class Mesh;
class ITexture;
class UIScreenEldMirror;
class UIWidgetImage;

typedef WBEvent TPersistence;

class EldritchGame : public IWBEventObserver
{
public:
	EldritchGame();
	~EldritchGame();

	void					Initialize();
	void					ShutDown();

	void					Tick();

	const SimpleString&		GetCurrentLevelName() const { return m_CurrentLevelName; }
	void					SetCurrentLevelName( const SimpleString& LevelName );

	TPersistence&			GetTravelPersistence() { return m_TravelPersistence; }
	static TPersistence&	StaticGetTravelPersistence();

	EldritchSaveLoad*		GetSaveLoad() const { return m_SaveLoad; }
	EldritchPersistence*	GetPersistence() const { return m_GenerationPersistence; }
	EldritchBank*			GetBank() const { return m_Bank; }
	EldritchMusic*			GetMusic() const { return m_Music; }

	void					Checkpoint() const;

	void					RefreshRTDependentSystems();
	void					CreatePostQuad();
	void					UpdateMirror();
	void					UpdateMinimap();
	Mesh*					GetPostQuad() const { return m_PostQuad; }
	void					SetColorGradingTexture( const SimpleString& TextureFilename );

	void					SetGamma( const float Gamma ) { m_Gamma = Gamma; }
	float					GetGamma() const { return m_Gamma; }

	void					SetFogParams( const float FogNear, const float FogFar, const SimpleString& FogTextureFilename );
	Vector2					GetFogParams() const { return m_FogParams; }
	ITexture*				GetFogTexture() const { return m_FogTexture; }

	void					SetCurrentMusic( const SimpleString& Music ) { m_CurrentMusic = Music; }

	// IWBEventObserver
	virtual void			HandleEvent( const WBEvent& Event );

	SimpleString			GetHubLevelName() const;
	bool					IsInHub() const;
	void					SetUIReturnToHubDisabled( const bool Disabled );
	void					RefreshUIReturnToHubEnabled();

	void					LaunchWebSite();

	void					LaunchRIPTweet();
	const SimpleString		GetRIPDamage();
	const SimpleString		GetRIPLevel();

	// Helper function, because where else would it go
	static WBEntity*		GetPlayer();
	static Vector			GetPlayerLocation();
	static Vector			GetPlayerViewLocation();
	static Angles			GetPlayerViewOrientation();
	static bool				IsPlayerAlive();
	static bool				IsPlayerDisablingPause();
	static bool				IsPlayerVisible();

	// More helpers
	static UIScreenEldMirror*	GetMirrorScreen();
	static UIWidgetImage*		GetMinimapImage();

private:
	void					ClearTravelPersistence();

	void					RequestReturnToHub( const bool Restart, const bool FlushHub );
	void					RequestGoToNextLevel();
	void					RequestGoToPrevLevel();
	void					RequestGoToLevel( const SimpleString& NextLevel, const HashedString& NextWorldDef, const bool RestoreSpawnPoint );
	void					GoToLevel();

	SimpleString			DecorateWorldFileName( const SimpleString& LevelName ) const;

	bool					m_GoToLevelOnNextTick;
	bool					m_IsRestarting;
	bool					m_FlushHubOnRestart;
	bool					m_RestoreSpawnPoint;
	SimpleString			m_NextLevelName;
	HashedString			m_NextWorldDef;

	SimpleString			m_CurrentLevelName;

	TPersistence			m_TravelPersistence;		// Travel persistence propagates world state data between worlds

	EldritchSaveLoad*		m_SaveLoad;
	EldritchPersistence*	m_GenerationPersistence;	// Generation persistence saves progress beyond death
	EldritchBank*			m_Bank;
	EldritchMusic*			m_Music;

	float					m_Gamma;
	Mesh*					m_PostQuad;
	SimpleString			m_ColorGradingTexture;
	ITexture*				m_FogTexture;
	Vector2					m_FogParams;	// x = near, y = 1/(far-near)
	SimpleString			m_CurrentMusic;
};

#endif // ELDRITCHGAME_H