#ifndef UISCREENELDCREDITS_H
#define UISCREENELDCREDITS_H

#include "uiscreen.h"

class UIScreenEldCredits : public UIScreen
{
public:
	UIScreenEldCredits();
	virtual ~UIScreenEldCredits();

	DEFINE_UISCREEN_FACTORY( EldCredits );

	virtual ETickReturn	Tick( float DeltaTime, bool HasFocus );
	virtual void		Pushed();
};

#endif // UISCREENELDCREDITS_H