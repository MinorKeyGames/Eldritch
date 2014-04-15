#include "core.h"
#include "uimanagerframework.h"
#include "framework3d.h"

UIManagerFramework::UIManagerFramework( Framework3D* pFramework )
:	m_Framework( pFramework )
{
}

UIManagerFramework::~UIManagerFramework()
{
}

/*virtual*/ Window* UIManagerFramework::GetWindow() const
{
	return m_Framework->GetWindow();
}

/*virtual*/ Keyboard* UIManagerFramework::GetKeyboard() const
{
	return m_Framework->GetKeyboard();
}

/*virtual*/ Mouse* UIManagerFramework::GetMouse() const
{
	return m_Framework->GetMouse();
}

/*virtual*/ IUIInputMap* UIManagerFramework::GetUIInputMap() const
{
	return m_Framework->GetUIInputMap();
}

/*virtual*/ Clock* UIManagerFramework::GetClock() const
{
	return m_Framework->GetClock();
}

/*virtual*/ Display* UIManagerFramework::GetDisplay() const
{
	return m_Framework->GetDisplay();
}

/*virtual*/ IRenderer* UIManagerFramework::GetRenderer() const
{
	return m_Framework->GetRenderer();
}

/*virtual*/ IAudioSystem* UIManagerFramework::GetAudioSystem() const
{
	return m_Framework->GetAudioSystem();
}

/*virtual*/ LineBatcher* UIManagerFramework::GetLineBatcher() const
{
	return NULL;
}