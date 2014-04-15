#ifndef ELDRITCHFRAMEWORK_H
#define ELDRITCHFRAMEWORK_H

#include "framework3d.h"

class EldritchGame;
class EldritchWorld;
class WBEventManager;
class View;
class InputSystem;
class Vector;
class Angles;
class EldritchTools;
class IDataStream;
class EldritchSound3DListener;
class Mesh;
class EldritchTargetManager;
class XInputController;
class CheckForUpdates;

class EldritchFramework : public Framework3D
{
public:
	EldritchFramework();
	virtual ~EldritchFramework();

	// IWBEventObserver
	virtual void	HandleEvent( const WBEvent& Event );

	EldritchWorld*			GetWorld() const { return m_World; }
	EldritchGame*			GetGame() const { return m_Game; }
	InputSystem*			GetInputSystem() const { return m_InputSystem; }
	EldritchTargetManager*	GetTargetManager() const { return m_TargetManager; }
	XInputController*		GetController() const { return m_Controller; }

	virtual SimpleString	GetSaveLoadPath();

	View*			GetMainView() const { return m_MainView; }
	void			SetMainViewTransform( const Vector& Location, const Angles& Orientation );

	void			RegenerateWorld();
	void			GoToLevel( const HashedString& WorldDef );

	void			PrepareForLoad();
	void			InitializeTools();

	// Singleton accessor
	static EldritchFramework*	GetInstance();
	static void					SetInstance( EldritchFramework* const pFramework );

protected:
	virtual void	Initialize();
	virtual void	ShutDown();

	void			InitializePackages();

	void			InitializeDLC();
	void			InitializeWorld( const HashedString& WorldDef, const bool CreateWorld );
	void			ShutDownWorld();

	void			LoadPrefsConfig();
	void			WritePrefsConfig();

	void			RegisterForEvents();

	void			HandleUISliderEvent( const HashedString& SliderName, const float SliderValue );

	void			Pause();

	float			GetMouseSpeedFromSliderValue( const float SliderValue );
	float			GetSliderValueFromMouseSpeed( const float MouseSpeed );
	float			GetControllerSpeedFromSliderValue( const float SliderValue );
	float			GetSliderValueFromControllerSpeed( const float ControllerSpeed );
	float			GetBrightnessFromSliderValue( const float SliderValue );
	float			GetSliderValueFromBrightness( const float MouseSpeed );
	float			GetFOVFromSliderValue( const float SliderValue );
	float			GetSliderValueFromFOV( const float FOV );

	virtual bool	TickSim( float DeltaTime );
	virtual bool	TickGame( float DeltaTime );
	virtual void	OnUnpaused();
	virtual void	TickDevices();
	virtual bool	TickInput( float DeltaTime );
	virtual void	TickPausedInput();
	virtual void	TickRender();

	virtual void	GetInitialWindowTitle( SimpleString& WindowTitle );
	virtual void	GetInitialWindowIcon( uint& WindowIcon );
	virtual void	GetUIManagerDefinitionName( SimpleString& DefinitionName );
	virtual void	InitializeUIInputMap();
	virtual bool	UseClassicTargetManager() { return false; }
	virtual bool	ShowWindowASAP() { return false; }
	virtual void	InitializeAudioSystem();

	void			CreateBuckets();
	void			UpdateViews();
	void			CreateHUDView();
	void			CreateMirrorView();
	void			CreateMinimapView();
	virtual void	ToggleFullscreen();
	virtual void	SetResolution( const uint DisplayWidth, const uint DisplayHeight );;
	virtual void	RefreshDisplay( const bool Fullscreen, const uint DisplayWidth, const uint DisplayHeight );

	static void		OnSetRes( void* pUIElement, void* pVoid );
	static void		OnFadeFinished( void* pUIElement, void* pVoid );

private:
	EldritchGame*				m_Game;
	EldritchWorld*				m_World;
#if BUILD_DEV
	EldritchTools*				m_Tools;
#endif
	XInputController*			m_Controller;
	InputSystem*				m_InputSystem;

#if BUILD_WINDOWS
	CheckForUpdates*			m_CheckForUpdates;
#endif

	uint						m_DisplayWidth;
	uint						m_DisplayHeight;

	EldritchTargetManager*		m_TargetManager;

	View*						m_MainView;
	View*						m_FGView;
	View*						m_HUDView;
	View*						m_MirrorView;
	View*						m_MinimapView;

	EldritchSound3DListener*	m_Audio3DListener;
};

#endif // ELDRITCHFRAMEWORK_H