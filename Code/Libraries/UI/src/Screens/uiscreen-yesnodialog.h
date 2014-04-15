#ifndef UISCREENYESNODIALOG_H
#define UISCREENYESNODIALOG_H

#include "uiscreen.h"

class UIScreenYesNoDialog : public UIScreen
{
public:
	UIScreenYesNoDialog();
	virtual ~UIScreenYesNoDialog();

	DEFINE_UISCREEN_FACTORY( YesNoDialog );

	void SetParameters(
		bool							PauseGame,
		const SimpleString&				YesNoString,
		const SimpleString&				YesNoDynamicString,
		const HashedString&				YesEvent,
		const HashedString&				NoEvent,
		const SimpleString&				YesCommand,
		const SimpleString&				NoCommand,
		const SUICallback&				YesCallback,
		const SUICallback&				NoCallback,
		const Array<WBAction*>* const	pYesActions,
		const Array<WBAction*>* const	pNoActions );
};

#endif // UISCREENYESNODIALOG_H