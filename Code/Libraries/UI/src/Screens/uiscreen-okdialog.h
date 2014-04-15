#ifndef UISCREENOKDIALOG_H
#define UISCREENOKDIALOG_H

#include "uiscreen.h"

class UIScreenOKDialog : public UIScreen
{
public:
	UIScreenOKDialog();
	virtual ~UIScreenOKDialog();

	DEFINE_UISCREEN_FACTORY( OKDialog );

	void SetParameters(
		bool				PauseGame,
		const SimpleString&	OKString,
		const SimpleString&	OKDynamicString,
		const HashedString&	OKEvent,
		const SimpleString&	OKCommand,
		const SUICallback&	OKCallback );
};

#endif // UISCREENOKDIALOG_H