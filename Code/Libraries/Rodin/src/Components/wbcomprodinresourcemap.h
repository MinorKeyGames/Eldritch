#ifndef WBCOMPRODINRESOURCEMAP_H
#define WBCOMPRODINRESOURCEMAP_H

#include "wbcomponent.h"
#include "array.h"
#include "map.h"
#include "hashedstring.h"

class IRodinResourceUser;

class WBCompRodinResourceMap : public WBComponent
{
public:
	WBCompRodinResourceMap();
	virtual ~WBCompRodinResourceMap();

	DEFINE_WBCOMP( RodinResourceMap, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	// It's always ok for a user to try to claim a resource it already has.
	bool			ClaimResource( IRodinResourceUser* const pResourceUser, const HashedString& Resource, const bool ForceClaim );
	void			ReleaseResource( IRodinResourceUser* const pResourceUser, const HashedString& Resource );

private:
	typedef Array<IRodinResourceUser*> TResourceStack;
	typedef Map<HashedString, TResourceStack> TResourceMap;
	TResourceMap	m_ResourceMap;	// Transient
};

#endif // WBCOMPRODINRESOURCEMAP_H