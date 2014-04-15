#ifndef WBCOMPELDRITCH_H
#define WBCOMPELDRITCH_H

#include "wbcomponent.h"

class EldritchFramework;
class EldritchGame;
class EldritchWorld;

class WBEldritchComponent : public WBComponent
{
public:
	WBEldritchComponent();
	virtual ~WBEldritchComponent();

protected:
	EldritchFramework*	GetFramework() const;
	EldritchGame*		GetGame() const;
	EldritchWorld*		GetWorld() const;
};

#endif // WBCOMPELDRITCH_H