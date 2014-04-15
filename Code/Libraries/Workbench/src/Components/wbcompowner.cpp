#include "core.h"
#include "wbcompowner.h"
#include "idatastream.h"
#include "wbeventmanager.h"

WBCompOwner::WBCompOwner()
:	m_Owner()
{
}

WBCompOwner::~WBCompOwner()
{
}

void WBCompOwner::SetOwner( WBEntity* const pNewOwner )
{
	m_Owner = pNewOwner;
}

WBEntity* WBCompOwner::GetOwner() const
{
	return m_Owner.Get();
}

/*static*/ WBEntity* WBCompOwner::GetTopmostOwner( WBEntity* pEntity )
{
	if( !pEntity )
	{
		return NULL;
	}

	for(;;)
	{
		WBCompOwner* pOwner = GET_WBCOMP( pEntity, Owner );
		if( pOwner )
		{
			WBEntity* pOwnerEntity = pOwner->GetOwner();
			if( !pOwnerEntity )
			{
				// This entity has an Owner component, but it's null. Return this.
				return pEntity;
			}
			else
			{
				// Iterate up the chain.
				pEntity = pOwnerEntity;
			}
		}
		else
		{
			// This entity has no Owner component. Return this.
			return pEntity;
		}
	}
}

/*virtual*/ void WBCompOwner::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	// Bit of a hack; OnEquipped is sent from Eldritch code and this lives in Workbench.
	STATIC_HASHED_STRING( OnEquipped );
	STATIC_HASHED_STRING( SetOwner );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnEquipped )
	{
		STATIC_HASHED_STRING( Owner );
		WBEntity* const pOwner = Event.GetEntity( sOwner );
		SetOwner( pOwner );
	}
	else if( EventName == sSetOwner )
	{
		STATIC_HASHED_STRING( NewOwner );
		WBEntity* const pNewOwner = Event.GetEntity( sNewOwner );

		SetOwner( pNewOwner );
	}
}

#define VERSION_EMPTY				0
#define VERSION_OWNER				1
#define VERSION_CURRENT				1

uint WBCompOwner::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;						// Version
	Size += sizeof( WBEntityRef );	// m_Owner

	return Size;
}

void WBCompOwner::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );
	Stream.Write( sizeof( WBEntityRef ), &m_Owner );
}

void WBCompOwner::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_OWNER )
	{
		Stream.Read( sizeof( WBEntityRef ), &m_Owner );
	}
}

void WBCompOwner::Report() const
{
	Super::Report();

	WBEntity* const pOwner = m_Owner.Get();
	PRINTF( WBPROPERTY_REPORT_PREFIX "Owner: %s\n", pOwner ? pOwner->GetUniqueName().CStr() : "NULL" );
}