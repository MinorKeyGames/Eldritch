#include "core.h"
#include "wbcompeldfootsteps.h"
#include "configmanager.h"
#include "mathcore.h"
#include "wbeventmanager.h"

WBCompEldFootsteps::WBCompEldFootsteps()
:	m_FootstepDistanceSq( 0.0f )
,	m_LastFootstepLocation()
#if BUILD_DEV
,	m_TEMPHACKFootstepsDisabled( false )
#endif
{
}

WBCompEldFootsteps::~WBCompEldFootsteps()
{
}

/*virtual*/ void WBCompEldFootsteps::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( FootstepDistance );
	m_FootstepDistanceSq = Square( ConfigManager::GetInheritedFloat( sFootstepDistance, 0.0f, sDefinitionName ) );
}

/*virtual*/ void WBCompEldFootsteps::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnMoved );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnMoved )
	{
#if BUILD_DEV
		if( m_TEMPHACKFootstepsDisabled )
		{
			return;
		}
#endif

		STATIC_HASHED_STRING( IsLanded );
		const bool IsLanded = Event.GetBool( sIsLanded );

		STATIC_HASHED_STRING( Location );
		const Vector Location = Event.GetVector( sLocation );

		const float DistanceSq = ( Location - m_LastFootstepLocation ).LengthSquared();
		if( IsLanded && DistanceSq >= m_FootstepDistanceSq )
		{
			m_LastFootstepLocation = Location;

			WB_MAKE_EVENT( OnFootstep, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), OnFootstep, GetEntity() );
		}
	}
}