#ifndef UIMANAGERFRAMEWORK_H
#define UIMANAGERFRAMEWORK_H

#include "Common/uimanagercommon.h"

class Framework3D;

class UIManagerFramework : public UIManagerCommon
{
public:
	UIManagerFramework( Framework3D* pFramework );
	virtual ~UIManagerFramework();

	virtual class Window*		GetWindow() const;
	virtual class Keyboard*		GetKeyboard() const;
	virtual class Mouse*		GetMouse() const;
	virtual class IUIInputMap*	GetUIInputMap() const;
	virtual class Clock*		GetClock() const;
	virtual class Display*		GetDisplay() const;
	virtual class IRenderer*	GetRenderer() const;
	virtual class IAudioSystem*	GetAudioSystem() const;
	virtual class LineBatcher*	GetLineBatcher() const;

protected:
	Framework3D*	m_Framework;
};

#endif // UIMANAGERGAME_H