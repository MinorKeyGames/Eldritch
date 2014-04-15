#ifndef UIINPUTMAPELDRITCH_H
#define UIINPUTMAPELDRITCH_H

#include "iuiinputmap.h"

class EldritchFramework;

class UIInputMapEldritch : public IUIInputMap
{
public:
	UIInputMapEldritch( EldritchFramework* const pFramework );
	virtual ~UIInputMapEldritch();

	virtual bool OnNext();
	virtual bool OnPrevious();
	virtual bool OnUp();
	virtual bool OnDown();
	virtual bool OnLeft();
	virtual bool OnRight();
	virtual bool OnAccept();
	virtual bool OnCancel();

protected:
	EldritchFramework*	m_Framework;
};

#endif // UIINPUTMAPELDRITCH_H