#ifndef WBCOMPPEMAP_H
#define WBCOMPPEMAP_H

#include "wbcomponent.h"
#include "wbpe.h"
#include "map.h"

class WBCompPEMap : public WBComponent
{
public:
	WBCompPEMap();
	virtual ~WBCompPEMap();

	DEFINE_WBCOMP( PEMap, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	WBPE*			GetPE( const HashedString& Name ) const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	Map<HashedString, WBPE*>	m_PEMap;
};

#endif // WBCOMPPEMAP_H