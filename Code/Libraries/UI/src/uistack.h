#ifndef UISTACK_H
#define UISTACK_H

#include "array.h"

class UIScreen;

class UIStack
{
public:
	UIStack();
	~UIStack();

	bool		HasFocus();
	UIScreen*	GetFocus();
	bool		PausesGame();
	void		Tick( float MachineDeltaTime, float GameDeltaTime, bool Render );
	void		Render();

	bool		IsOnStack( UIScreen* const pScreen );
	void		Push( UIScreen* pScreen );
	void		Repush( UIScreen* pScreen );	// Push a screen that may already be on the stack; if it is, the existing instance is floated to the top
	void		Remove( UIScreen* pScreen );	// Remove a screen regardless of its place in the stack.
	void		Replace( UIScreen* pOldScreen, UIScreen* pNewScreen );	// Replace a screen in the stack with another screen.
	void		Pop();
	UIScreen*	Top();
	void		Clear();	// Reset the UI stack, calling each screen's Popped method

	void		SetFadeOverlay( UIScreen* pScreen );
	void		SetDebugOverlay( UIScreen* pScreen );

private:
	struct SStackScreen
	{
		SStackScreen()
		:	m_Screen( NULL )
		,	m_Popped( false )
		{
		}

		UIScreen*	m_Screen;
		bool		m_Popped;
	};

	Array<SStackScreen>	m_ScreenStack;
	UIScreen*			m_FadeOverlay;	// Always draws over stack but under debug overlay
	UIScreen*			m_DebugOverlay;	// Always draws over the stack

#if BUILD_DEBUG
	mutable bool		m_IteratingStack;
#endif
};

#endif // UISTACK_H