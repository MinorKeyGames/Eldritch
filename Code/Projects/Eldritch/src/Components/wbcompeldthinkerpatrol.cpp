#include "core.h"
#include "wbcompeldthinkerpatrol.h"
#include "Components/wbcomprodinknowledge.h"
#include "Components/wbcomprodinblackboard.h"
#include "Components/wbcompeldtransform.h"
#include "configmanager.h"

WBCompEldThinkerPatrol::WBCompEldThinkerPatrol()
:	m_OutputBlackboardKey()
{
}

WBCompEldThinkerPatrol::~WBCompEldThinkerPatrol()
{
}

/*virtual*/ void WBCompEldThinkerPatrol::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( OutputBlackboardKey );
	m_OutputBlackboardKey = ConfigManager::GetInheritedHash( sOutputBlackboardKey, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBCompEldThinkerPatrol::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	WBEntity* const					pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompEldTransform* const		pTransform	= pEntity->GetTransformComponent<WBCompEldTransform>();
	DEVASSERT( pTransform );

	WBCompRodinKnowledge* const		pKnowledge	= GET_WBCOMP( pEntity, RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompRodinBlackboard* const	pBlackboard	= GET_WBCOMP( pEntity, RodinBlackboard );
	ASSERT( pBlackboard );

	const Vector	CurrentLocation	= pTransform->GetLocation();

	// Select the furthest patrol point that we know about.
	WBEntity*		pFurthestPatrol	= NULL;
	float			FurthestDistSq	= -1.0f;

	const WBCompRodinKnowledge::TKnowledgeMap& KnowledgeMap = pKnowledge->GetKnowledgeMap();
	FOR_EACH_MAP( KnowledgeIter, KnowledgeMap, WBEntityRef, WBCompRodinKnowledge::TKnowledge )
	{
		WBEntity*								pKnowledgeEntity	= KnowledgeIter.GetKey().Get();
		const WBCompRodinKnowledge::TKnowledge&	Knowledge			= KnowledgeIter.GetValue();

		if( !pKnowledgeEntity )
		{
			continue;
		}

		// Filter out knowledge entities that aren't patrol markup.
		STATIC_HASHED_STRING( KnowledgeType );
		STATIC_HASHED_STRING( Patrol );
		if( Knowledge.GetHash( sKnowledgeType ) != sPatrol )
		{
			continue;
		}

		WBCompEldTransform* const				pKnowledgeTransform	= pKnowledgeEntity->GetTransformComponent<WBCompEldTransform>();
		ASSERT( pKnowledgeTransform );

		const float DistSq = ( pKnowledgeTransform->GetLocation() - CurrentLocation ).LengthSquared();
		if( DistSq > FurthestDistSq )
		{
			FurthestDistSq = DistSq;
			pFurthestPatrol = pKnowledgeEntity;
		}
	}

	pBlackboard->SetEntity( m_OutputBlackboardKey, pFurthestPatrol );
}