#ifndef WBENTITYREF_H
#define WBENTITYREF_H

#include "wbentity.h"
#include "wbworld.h"

class WBEntityRef
{
public:
	WBEntityRef( uint UID = 0 ) : m_UID( UID ) {}
	WBEntityRef( const WBEntity* const pEntity ) : m_UID( pEntity ? pEntity->GetUID() : 0 ) {}

	bool operator==( const WBEntityRef& OtherRef ) const { return m_UID == OtherRef.m_UID; }
	bool operator==( uint OtherUID ) const { return m_UID == OtherUID; }
	bool operator!=( const WBEntityRef& OtherRef ) const { return m_UID != OtherRef.m_UID; }
	bool operator!=( uint OtherUID ) const { return m_UID != OtherUID; }

	WBEntityRef& operator=( uint OtherUID ) { m_UID = OtherUID; return *this; }
	WBEntityRef& operator=( const WBEntityRef& OtherRef ) { m_UID = OtherRef.m_UID; return *this; }

	operator unsigned long() const { return m_UID; }

	// These three functions do the same thing
	WBEntity* operator*() const { return WBWorld::GetInstance()->GetEntity( m_UID ); }
	WBEntity* operator()() const { return WBWorld::GetInstance()->GetEntity( m_UID ); }
	WBEntity* Get() const { return WBWorld::GetInstance()->GetEntity( m_UID ); }

private:
	uint m_UID;
};

#endif // WBENTITYREF_H