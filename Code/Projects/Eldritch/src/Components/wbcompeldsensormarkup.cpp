#include "core.h"
#include "wbcompeldsensormarkup.h"
#include "wbcompeldtransform.h"
#include "Components/wbcomprodinknowledge.h"
#include "configmanager.h"
#include "wbcomponentarrays.h"
#include "wbcompeldmarkup.h"
#include "mathcore.h"

WBCompEldSensorMarkup::WBCompEldSensorMarkup()
:	m_Radius( 0.0f )
{
}

WBCompEldSensorMarkup::~WBCompEldSensorMarkup()
{
}

/*virtual*/ void WBCompEldSensorMarkup::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Markup );
	m_Markup = ConfigManager::GetInheritedHash( sMarkup, HashedString::NullString, sDefinitionName );

	STATICHASH( Radius );
	m_Radius = ConfigManager::GetInheritedFloat( sRadius, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldSensorMarkup::PollTick( const float DeltaTime ) const
{
	Unused( DeltaTime );

	const Array<WBCompEldMarkup*>* pMarkupComponents = WBComponentArrays::GetComponents<WBCompEldMarkup>();
	if( !pMarkupComponents )
	{
		return;
	}

	WBEntity* const				pEntity			= GetEntity();
	DEVASSERT( pEntity );

	WBCompRodinKnowledge* const	pKnowledge		= GET_WBCOMP( pEntity, RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompEldTransform* const pTransform		= GetEntity()->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	const Vector				CurrentLocation	= pTransform->GetLocation();
	const float					RadiusSq		= Square( m_Radius );

	const uint NumMarkups = pMarkupComponents->Size();
	for( uint MarkupIndex = 0; MarkupIndex < NumMarkups; ++MarkupIndex )
	{
		WBCompEldMarkup* const	pMarkup					= ( *pMarkupComponents )[ MarkupIndex ];
		ASSERT( pMarkup );

		// Only consider the desired type of markup
		if( pMarkup->GetMarkup() != m_Markup )
		{
			continue;
		}

		WBEntity* const			pMarkupEntity			= pMarkup->GetEntity();
		ASSERT( pMarkupEntity );

		WBCompEldTransform* const	pMarkupTransform	= pMarkupEntity->GetTransformComponent<WBCompEldTransform>();

		// Distance check
		const float DistanceSq = ( pMarkupTransform->GetLocation() - CurrentLocation ).LengthSquared();
		if( DistanceSq > RadiusSq )
		{
			continue;
		}

		// Update knowledge with this patrol
		WBCompRodinKnowledge::TKnowledge& Knowledge = pKnowledge->UpdateEntity( pMarkupEntity );

		STATIC_HASHED_STRING( DistanceSq );
		Knowledge.SetFloat( sDistanceSq, DistanceSq );

		STATIC_HASHED_STRING( LastKnownLocation );
		Knowledge.SetVector( sLastKnownLocation, pMarkupTransform->GetLocation() );
		ASSERT( !pMarkupTransform->GetLocation().IsZero() );

		STATIC_HASHED_STRING( KnowledgeType );
		Knowledge.SetHash( sKnowledgeType, m_Markup );
	}
}