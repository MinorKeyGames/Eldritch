#include "core.h"
#include "uistack.h"
#include "uiscreen.h"

#if BUILD_DEBUG
#define BEGIN_ITERATING_STACK do{ m_IteratingStack = true; } while(0)
#define END_ITERATING_STACK do{ m_IteratingStack = false; } while(0)
#define CHECK_ITERATING_STACK do{ DEBUGASSERT( !m_IteratingStack ); } while(0)
#else
#define BEGIN_ITERATING_STACK DoNothing
#define END_ITERATING_STACK DoNothing
#define CHECK_ITERATING_STACK DoNothing
#endif

UIStack::UIStack()
:	m_ScreenStack()
,	m_FadeOverlay( NULL )
,	m_DebugOverlay( NULL )
#if BUILD_DEBUG
,	m_IteratingStack( false )
#endif
{
	m_ScreenStack.SetDeflate( false );
}

UIStack::~UIStack()
{
}

bool UIStack::HasFocus()
{
	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( StackScreen.m_Screen->TakesFocus() )
		{
			return true;
		}
	}
	return false;
}

UIScreen* UIStack::GetFocus()
{
	FOR_EACH_ARRAY_REVERSE( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( StackScreen.m_Screen->TakesFocus() )
		{
			return StackScreen.m_Screen;
		}
	}
	return NULL;
}

bool UIStack::PausesGame()
{
	XTRACE_FUNCTION;

	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( StackScreen.m_Screen->PausesGame() )
		{
			return true;
		}
	}
	return false;
}

void UIStack::Tick( float MachineDeltaTime, float GameDeltaTime, bool Render )
{
	XTRACE_FUNCTION;

	UIScreen* const pFocusScreen = GetFocus();

	BEGIN_ITERATING_STACK;
	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		UIScreen* const pScreen = StackScreen.m_Screen;
		ASSERT( pScreen );

		if( pScreen->m_Hidden )
		{
			continue;
		}

		const bool HasFocus = ( pFocusScreen == pScreen );
		const UIScreen::ETickReturn TickReturn = pScreen->Tick( pScreen->m_GameTick ? GameDeltaTime : MachineDeltaTime, HasFocus );

		if( Render )
		{
			pScreen->Render( HasFocus );
		}

		if( pScreen->AllowClose() && TickReturn == UIScreen::ETR_Close )
		{
			pScreen->Closed();
			pScreen->Popped();

			// Re-get the reference because the array may have been resized and reallocated under us.
			SStackScreen& StackScreen = ScreenIter.GetValue();
			StackScreen.m_Popped = true;
		}
	}
	END_ITERATING_STACK;

	FOR_EACH_ARRAY_NOINCR( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();
		if( StackScreen.m_Popped )
		{
			m_ScreenStack.Remove( ScreenIter );
		}
		else
		{
			++ScreenIter;
		}
	}

	// Draw fade overlay (after everything else so it's always on top of the UI)
	if( m_FadeOverlay )
	{
		m_FadeOverlay->Tick( MachineDeltaTime, false );

		if( Render )
		{
			m_FadeOverlay->Render( false );
		}
	}

	// Draw debug overlay--a single special-case screen that's always on top and
	// never has focus (and isn't part of the stack for any other uses).
	if( m_DebugOverlay )
	{
		m_DebugOverlay->Tick( MachineDeltaTime, false );

		if( Render )
		{
			m_DebugOverlay->Render( false );
		}
	}
}

void UIStack::Render()
{
	XTRACE_FUNCTION;

	UIScreen* const pFocusScreen = GetFocus();

	BEGIN_ITERATING_STACK;
	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		UIScreen* const pScreen = StackScreen.m_Screen;

		if( pScreen->m_Hidden )
		{
			continue;
		}
		
		const bool HasFocus = ( pFocusScreen == pScreen );
		pScreen->Render( HasFocus );
	}
	END_ITERATING_STACK;

	// Draw fade overlay (after everything else so it's always on top of the UI)
	if( m_FadeOverlay )
	{
		m_FadeOverlay->Render( false );
	}

	// Draw debug overlay--a single special-case screen that's always on top and
	// never has focus (and isn't part of the stack for any other uses).
	if( m_DebugOverlay )
	{
		m_DebugOverlay->Render( false );
	}
}

bool UIStack::IsOnStack( UIScreen* const pScreen )
{
	ASSERT( pScreen );

	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( pScreen == StackScreen.m_Screen )
		{
			return true;
		}
	}

	return false;
}

void UIStack::Push( UIScreen* pScreen )
{
	if( !pScreen )
	{
		return;
	}

#if BUILD_DEBUG
	// Warn if pushing a screen that's already on the stack
	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		ASSERTDESC( pScreen != StackScreen.m_Screen, "Warning: Pushing a screen that is already on the UI stack! Consider using Repush()." );
	}
#endif

	ASSERT( pScreen );
	if( pScreen )
	{
		SStackScreen StackScreen;
		StackScreen.m_Screen = pScreen;
		m_ScreenStack.PushBack( StackScreen );
		pScreen->Pushed();
	}
}

void UIStack::Repush( UIScreen* pScreen )
{
	ASSERT( pScreen );

	// Remove the screen without calling Popped on it.
	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( pScreen == StackScreen.m_Screen )
		{
			StackScreen.m_Popped = true;
			break;
		}
	}

	Push( pScreen );
}

void UIStack::Remove( UIScreen* pScreen )
{
	ASSERT( pScreen );

	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( pScreen == StackScreen.m_Screen )
		{
			pScreen->Popped();
			StackScreen.m_Popped = true;
			break;
		}
	}
}

void UIStack::Replace( UIScreen* pOldScreen, UIScreen* pNewScreen )
{
	CHECK_ITERATING_STACK;

	ASSERT( pOldScreen );
	ASSERT( pNewScreen );

	FOR_EACH_ARRAY( ScreenIter, m_ScreenStack, SStackScreen )
	{
		SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		if( StackScreen.m_Screen == pOldScreen )
		{
			pOldScreen->Popped();
			pNewScreen->Pushed();
			StackScreen.m_Screen = pNewScreen;
			break;
		}
	}
}

void UIStack::Pop()
{
	FOR_EACH_ARRAY_REVERSE( ScreenIter, m_ScreenStack, SStackScreen )
	{
		SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		StackScreen.m_Screen->Popped();
		StackScreen.m_Popped = true;
		break;
	}
}

UIScreen* UIStack::Top()
{
	FOR_EACH_ARRAY_REVERSE( ScreenIter, m_ScreenStack, SStackScreen )
	{
		const SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		return StackScreen.m_Screen;
	}

	return NULL;
}

void UIStack::Clear()
{
	FOR_EACH_ARRAY_REVERSE( ScreenIter, m_ScreenStack, SStackScreen )
	{
		SStackScreen& StackScreen = ScreenIter.GetValue();

		if( StackScreen.m_Popped )
		{
			continue;
		}

		StackScreen.m_Screen->Popped();
		StackScreen.m_Popped = true;
	}
}

void UIStack::SetFadeOverlay( UIScreen* pScreen )
{
	m_FadeOverlay = pScreen;
}

void UIStack::SetDebugOverlay( UIScreen* pScreen )
{
	m_DebugOverlay = pScreen;
}