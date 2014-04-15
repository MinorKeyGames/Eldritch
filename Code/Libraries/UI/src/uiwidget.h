#ifndef UIWIDGET_H
#define UIWIDGET_H

#include "ui.h"
#include "simplestring.h"
#include "vector2.h"
#include "vector4.h"

class UIManager;
class UIScreen;
class WBAction;
struct SRect;

#define DEFINE_UIWIDGET_FACTORY( type ) static class UIWidget* Factory() { return new UIWidget##type; }
typedef class UIWidget* ( *UIWidgetFactoryFunc )( void );

class UIWidget
{
public:
	enum EWidgetOrigin
	{
		EWO_TopLeft,
		EWO_TopCenter,
		EWO_TopRight,
		EWO_CenterLeft,
		EWO_Center,
		EWO_CenterRight,
		EWO_BottomLeft,
		EWO_BottomCenter,
		EWO_BottomRight,
	};

	UIWidget();
	virtual ~UIWidget();

	virtual void		Render( bool HasFocus );
	virtual void		UpdateRender();				// Pushes widget changes to renderable
	virtual void		InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void		GetBounds( SRect& OutBounds );
	virtual void		OnTrigger();
	virtual void		Released();
	virtual void		Drag( float X, float Y );
	virtual bool		HandleInput();
	virtual void		GetFocus();
	virtual void		Refresh();
	virtual void		Tick( const float DeltaTime );

	bool				HasFocus() const;

	void				Reinitialize();

	int					OverrideFocusUp( int FocusShift );
	int					OverrideFocusDown( int FocusShift );
	int					OverrideFocusLeft( int FocusShift );
	int					OverrideFocusRight( int FocusShift );

	bool				IsHidden() const { return m_Hidden; }
	void				SetHidden( const bool Hidden );
	void				Show();
	void				Hide() { m_Hidden = true; }

	bool				IsDisabled() const { return m_IsDisabled; }
	void				SetDisabled( const bool Disabled );

	// X and Y are screen values in range [0,1]
	void				SetLocation( const float X, const float Y );

	void				SetAlpha( const float Alpha ){ m_Color.a = Alpha; }

	void				ClearActions();

	// For maintaining state when reinitializing widget.
	virtual void		PushState();
	virtual void		PullState();

	virtual float		GetX() { return m_TopLeft.x; }
	virtual float		GetY() { return m_TopLeft.y; }
	virtual float		GetWidth() { return 0.0f; }
	virtual float		GetHeight() { return 0.0f; }

	UIScreen*			GetOwnerScreen() const { return m_OwnerScreen; }
	void				SetOwnerScreen( UIScreen* const pScreen ) { m_OwnerScreen = pScreen; }

	void				GetOriginFromString( const SimpleString& Origin );
	void				GetPositionFromOrigin( const float X, const float Y, const float Width, const float Height );

	void				AdjustDimensionsToParent(
		float& X, float& Y, float& Width, float& Height,
		const float ParentX, const float ParentY, const float ParentWidth, const float ParentHeight );

	Vector4				GetHighlightColor() const;

	float				GetPixelGridOffset() const;

	SimpleString		m_Name;
	SimpleString		m_Archetype;
	int					m_RenderPriority;	// Low numbers draw first, so high numbers draw on top
	bool				m_CanBeFocused;
	bool				m_RenderInWorld;
	bool				m_IsDisabled;
	int					m_FocusOrder;
	UIScreen*			m_OwnerScreen;

	int					m_FocusShiftUp;
	int					m_FocusShiftDown;
	int					m_FocusShiftLeft;
	int					m_FocusShiftRight;

	HashedString		m_EventName;	// Event that gets fired when this widget is triggered
	SimpleString		m_Command;		// Literal console command that is issued when this widget is triggered
	SUICallback			m_Callback;		// Callback that is invoked when this widget is triggered
	SimpleString		m_FocusSound;	// Sound definition that is played when this widget gets focus
	SimpleString		m_TriggerSound;	// Sound definition that is played when this widget is triggered
	Array<WBAction*>	m_Actions;		// Actions that are executed when this widget is triggered
	bool				m_OwnsActions;	// Does this widget own its m_Actions?

	bool				m_Hidden;
	bool				m_Hidden_SavedState;
	Vector4				m_Color;
	Vector4				m_Highlight;
	Vector4				m_DisabledColor;
	Vector2				m_TopLeft;
	Vector2				m_Velocity;
	bool				m_AllowNegativeOrigin;	// If true, negative origin is used literally instead of being used as relative to parent's high bound. Useful for spacers.
	EWidgetOrigin		m_Origin;
	UIWidget*			m_OriginParent;	// Must be defined before this widget, obviously

	bool				m_UsePulsingHighlight;
	float				m_PulsingHighlightMul;
	float				m_PulsingHighlightAdd;
	float				m_PulsingHighlightTimeScalar;

#if BUILD_DEBUG
	debug_int64_t		m_LastRenderedTime;
#endif

	static void			SetUIManager( UIManager* pManager );
	static UIManager*	m_UIManager;
};

#endif // UIWIDGET_H