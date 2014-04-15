#ifndef UISCREENFADE_H
#define UISCREENFADE_H

// Fade is a permanent overlay on the screen used to do fades in and out.
// Like debug overlays, it is never popped, is always "on top" of the stack
// (but underneath debug overlay), and should never have focus or pause.

#include "uiscreen.h"
#include "vector4.h"
#include "interpolator.h"

class UIScreenFade : public UIScreen
{
public:
	UIScreenFade();
	UIScreenFade( const SimpleString& DefinitionName );
	virtual ~UIScreenFade();

	DEFINE_UISCREEN_FACTORY( Fade );

	virtual ETickReturn	Tick( float DeltaTime, bool HasFocus );

	void	Fade( const Vector4& StartColor, const Vector4& EndColor, const float Duration );
	void	SetFadeCallback( const SUICallback& FadeCallback );

private:
	Interpolator< Vector4 >	m_FadeInterpolator;
	SUICallback				m_FadeFinishedCallback;
};

#endif // UISCREENFADE_H