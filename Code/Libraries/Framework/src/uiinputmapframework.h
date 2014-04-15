#ifndef UIINPUTMAPFRAMEWORK_H
#define UIINPUTMAPFRAMEWORK_H

#include "iuiinputmap.h"

class Framework3D;

class UIInputMapFramework : public IUIInputMap
{
public:
	UIInputMapFramework( Framework3D* pFramework );
	virtual ~UIInputMapFramework();

	virtual bool OnNext();
	virtual bool OnPrevious();
	virtual bool OnUp();
	virtual bool OnDown();
	virtual bool OnLeft();
	virtual bool OnRight();
	virtual bool OnAccept();
	virtual bool OnCancel();

protected:
	Framework3D*	m_Framework;
};

#endif // UIINPUTMAPFRAMEWORK_H