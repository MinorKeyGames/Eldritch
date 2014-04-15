#ifndef WBCOMPLABEL_H
#define WBCOMPLABEL_H

#include "wbcomponent.h"

class WBCompLabel : public WBComponent
{
public:
	WBCompLabel();
	virtual ~WBCompLabel();

	DEFINE_WBCOMP( Label, WBComponent );

	virtual bool	BelongsInComponentArray() { return true; }
	virtual int		GetTickOrder() { return ETO_NoTick; }

	HashedString	GetLabel() { return m_Label; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_Label;	// Config
};

#endif // WBCOMPLABEL_H