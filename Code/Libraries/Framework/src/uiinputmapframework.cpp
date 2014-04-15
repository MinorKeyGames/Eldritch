#include "core.h"
#include "uiinputmapframework.h"
#include "framework3d.h"
#include "keyboard.h"

UIInputMapFramework::UIInputMapFramework( Framework3D* pFramework )
:	m_Framework( pFramework )
{
}

UIInputMapFramework::~UIInputMapFramework()
{
}

bool UIInputMapFramework::OnNext()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Tab ) &&
		!( m_Framework->GetKeyboard()->IsHigh( Keyboard::EB_LeftControl ) ||
		   m_Framework->GetKeyboard()->IsHigh( Keyboard::EB_RightControl ) ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnPrevious()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Tab ) &&
		( m_Framework->GetKeyboard()->IsHigh( Keyboard::EB_LeftControl ) ||
		  m_Framework->GetKeyboard()->IsHigh( Keyboard::EB_RightControl ) ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnUp()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Up ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnDown()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Down ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnLeft()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Left ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnRight()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Right ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnAccept()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Enter ) &&
		!( m_Framework->GetKeyboard()->IsHigh( Keyboard::EB_LeftAlt ) ||
		   m_Framework->GetKeyboard()->IsHigh( Keyboard::EB_RightAlt ) ) )
	{
		return true;
	}

	return false;
}

bool UIInputMapFramework::OnCancel()
{
	if( m_Framework->GetKeyboard()->OnRise( Keyboard::EB_Escape ) )
	{
		return true;
	}

	return false;
}