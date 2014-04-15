#ifndef UISCREENWAITDIALOG_H
#define UISCREENWAITDIALOG_H

#include "uiscreen.h"

class UIScreenWaitDialog : public UIScreen
{
public:
	UIScreenWaitDialog();
	virtual ~UIScreenWaitDialog();

	DEFINE_UISCREEN_FACTORY( WaitDialog );

	virtual ETickReturn	Tick( float DeltaTime, bool HasFocus );

	void SetParameters(
		bool				PauseGame,
		const SimpleString&	WaitString,
		const SimpleString&	WaitDynamicString );
};

#endif // UISCREENWAITDIALOG_H