#ifndef UISCREEN_H
#define UISCREEN_H

#include "ui.h"
#include "array.h"
#include "simplestring.h"
#include "map.h"

class Game;
class UIManager;
class UIWidget;
class WBAction;

#define DEFINE_UISCREEN_FACTORY( type ) static class UIScreen* Factory() { return new UIScreen##type; }
typedef class UIScreen* ( *UIScreenFactoryFunc )( void );

class UIScreen
{
public:
	UIScreen();
	UIScreen( const SimpleString& DefinitionName );
	virtual ~UIScreen();

	enum ETickReturn
	{
		ETR_None,
		ETR_Close,
	};

	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	void				Reinitialize();

	virtual bool		TakesFocus();
	virtual bool		PausesGame();
	virtual ETickReturn	Tick( float DeltaTime, bool HasFocus );
	void				TickMouse();

	virtual void		Pushed();
	virtual void		Popped();
	virtual void		Closed();	// Only invoked when the screen is manually closed, not any other time it is popped

	bool				AllowClose() const { return m_AllowClose; }

	void				AddWidget( UIWidget* pNewWidget );
	virtual void		Render( bool HasFocus );
	void				UpdateRender();
	void				SortWidgets();
	void				RefreshWidgets();
	void				TickWidgets( const float DeltaTime );

	void				FlushWidgets();

	UIWidget*				GetWidget( const HashedString& Name );
	template<class C> C*	GetWidget( const HashedString& Name ) { return static_cast<C*>( GetWidget( Name ) ); }

	UIWidget*			GetFocusedWidget() const { return m_FocusedWidget; }

	void				SetFocus( UIWidget* pWidget );
	void				FocusNext();
	void				FocusPrevious();
	void				FocusUp();
	void				FocusDown();
	void				FocusLeft();
	void				FocusRight();
	void				ShiftFocus( int Amount );
	uint				GetFocusIndex();

	Map<HashedString, UIWidget*>	m_Widgets;			// All the widgets, accessible by name
	Array< UIWidget* >				m_RenderWidgets;	// All the widgets, sorted in render order
	Array< UIWidget* >				m_FocusWidgets;		// Just the usable widgets, sorted in cycling order
	UIWidget*						m_FocusedWidget;

	SimpleString		m_Name;

	bool				m_TakesFocus;
	bool				m_PausesGame;
	bool				m_GameTick;

	int					m_FocusXShift;
	int					m_FocusYShift;

	// Cache the mouse position so we only consider it when it moves
	int					m_LastMousePosX;
	int					m_LastMousePosY;
	UIWidget*			m_ClickedWidget;	// The widget that was clicked on, to check that the release is on the same widget

	SimpleString		m_PushedSound;		// Sound definition that is played when this screen is pushed
	SimpleString		m_PoppedSound;		// Sound definition that is played when this screen is popped

	bool				m_OnStack;
	bool				m_Hidden;			// Programmatically hide screens on the stack; hidden screens are neither drawn nor ticked

	bool				m_SeizesMouse;		// Config
	bool				m_HasSeizedMouse;	// Transient

	Array<WBAction*>	m_PushedActions;	// Actions that are executed when this widget is pushed

	bool				m_AllowClose;
	Array<WBAction*>	m_ClosedActions;	// Actions that are executed when this widget is manually closed

	static void			SetUIManager( UIManager* pManager );
	static UIManager*	m_UIManager;

	static void			UpdateMouseButtonsSwapped();
	static bool			m_MouseButtonsSwapped;
};

#endif // UISCREEN_H