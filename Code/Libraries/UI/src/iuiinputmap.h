#ifndef IUIINPUTMAP_H
#define IUIINPUTMAP_H

class IUIInputMap
{
public:
	virtual ~IUIInputMap() {}

	virtual bool OnNext() = 0;
	virtual bool OnPrevious() = 0;
	virtual bool OnUp() = 0;
	virtual bool OnDown() = 0;
	virtual bool OnLeft() = 0;
	virtual bool OnRight() = 0;
	virtual bool OnAccept() = 0;
	virtual bool OnCancel() = 0;
};

#endif // IUIINPUTMAP_H