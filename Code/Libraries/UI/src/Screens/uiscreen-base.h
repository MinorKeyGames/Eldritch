#ifndef UISCREENBASE_H
#define UISCREENBASE_H

#include "uiscreen.h"

class UIScreenBase : public UIScreen
{
public:
	UIScreenBase();
	virtual ~UIScreenBase();

	DEFINE_UISCREEN_FACTORY( Base );
};

#endif // UISCREENBASE_H