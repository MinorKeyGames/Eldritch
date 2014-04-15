#include "core.h"
#include "wbcompeldthinkerhearwormtarget.h"
#include "Components/wbcomprodinknowledge.h"
#include "Components/wbcomprodinblackboard.h"
#include "wbcompeldfaction.h"
#include "wbcompeldhealth.h"
#include "configmanager.h"

WBCompEldThinkerHearWormTarget::WBCompEldThinkerHearWormTarget()
:	m_OutputAlarmTargetBlackboardKey()
,	m_OutputWatchTargetBlackboardKey()
,	m_AlarmTargetScoreThreshold( 0.0f )
,	m_WatchTargetScoreThreshold( 0.0f )
{
}

WBCompEldThinkerHearWormTarget::~WBCompEldThinkerHearWormTarget()
{
}

/*virtual*/ void WBCompEldThinkerHearWormTarget::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( OutputAlarmTargetBlackboardKey );
	m_OutputAlarmTargetBlackboardKey = ConfigManager::GetInheritedHash( sOutputAlarmTargetBlackboardKey, HashedString::NullString, sDefinitionName );

	STATICHASH( OutputWatchTargetBlackboardKey );
	m_OutputWatchTargetBlackboardKey = ConfigManager::GetInheritedHash( sOutputWatchTargetBlackboardKey, HashedString::NullString, sDefinitionName );

	STATICHASH( AlarmTargetScoreThreshold );
	m_AlarmTargetScoreThreshold = ConfigManager::GetInheritedFloat( sAlarmTargetScoreThreshold, 0.0f, sDefinitionName );

	STATICHASH( WatchTargetScoreThreshold );
	m_WatchTargetScoreThreshold = ConfigManager::GetInheritedFloat( sWatchTargetScoreThreshold, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldThinkerHearWormTarget::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	WBEntity* const					pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompRodinKnowledge* const		pKnowledge	= GET_WBCOMP( pEntity, RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompRodinBlackboard* const	pBlackboard	= GET_WBCOMP( pEntity, RodinBlackboard );
	ASSERT( pBlackboard );

	WBEntity*	pSelectedAlarmTarget		= NULL;
	float		SelectedAlarmTargetScore	= 0.0f;

	WBEntity*	pSelectedWatchTarget		= NULL;
	float		SelectedWatchTargetScore	= 0.0f;

	const WBCompRodinKnowledge::TKnowledgeMap& KnowledgeMap = pKnowledge->GetKnowledgeMap();
	FOR_EACH_MAP( KnowledgeIter, KnowledgeMap, WBEntityRef, WBCompRodinKnowledge::TKnowledge )
	{
		WBEntity* const							pKnowledgeEntity	= KnowledgeIter.GetKey().Get();
		const WBCompRodinKnowledge::TKnowledge&	Knowledge			= KnowledgeIter.GetValue();

		if( !pKnowledgeEntity )
		{
			continue;
		}

		// Filter out knowledge entities that aren't potential targets
		STATIC_HASHED_STRING( KnowledgeType );
		STATIC_HASHED_STRING( Target );
		const HashedString KnowledgeType = Knowledge.GetHash( sKnowledgeType );
		if( KnowledgeType != sTarget )
		{
			continue;
		}

		// Filter out dead entities
		WBCompEldHealth* const pHealth = GET_WBCOMP( pKnowledgeEntity, EldHealth );
		if( pHealth && pHealth->IsDead() )
		{
			continue;
		}

		// Filter out knowledge entities that are friendly to us
		const EldritchFactions::EFactionCon Con = WBCompEldFaction::GetCon( pEntity, pKnowledgeEntity );
		if( Con == EldritchFactions::EFR_Friendly )
		{
			continue;
		}

		// Filter out knowledge entities that are neutral and not yet regarded as hostile
		if( Con == EldritchFactions::EFR_Neutral )
		{
			STATIC_HASHED_STRING( RegardAsHostile );
			const bool RegardAsHostile = Knowledge.GetBool( sRegardAsHostile );
			if( !RegardAsHostile )
			{
				continue;
			}
		}

		float Score			= 0.0f;
		float HearingScore	= 1.0f;

		STATIC_HASHED_STRING( HearingCertainty );
		const float HearingCertainty = Knowledge.GetFloat( sHearingCertainty );
		HearingScore *= HearingCertainty;

		Score += HearingScore;

		if( Score >= m_AlarmTargetScoreThreshold && Score > SelectedAlarmTargetScore )
		{
			pSelectedAlarmTarget		= pKnowledgeEntity;
			SelectedAlarmTargetScore	= Score;
		}

		if( Score >= m_WatchTargetScoreThreshold && Score > SelectedWatchTargetScore )
		{
			pSelectedWatchTarget		= pKnowledgeEntity;
			SelectedWatchTargetScore	= Score;
		}
	}

	pBlackboard->SetEntity( m_OutputAlarmTargetBlackboardKey, pSelectedAlarmTarget );
	pBlackboard->SetEntity( m_OutputWatchTargetBlackboardKey, pSelectedWatchTarget );
}