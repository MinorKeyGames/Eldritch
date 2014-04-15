#include "core.h"
#include "wbcompeldthinkertarget.h"
#include "Components/wbcomprodinknowledge.h"
#include "Components/wbcomprodinblackboard.h"
#include "wbcompeldfaction.h"
#include "wbcompeldhealth.h"
#include "configmanager.h"

WBCompEldThinkerTarget::WBCompEldThinkerTarget()
:	m_OutputCombatTargetBlackboardKey()
,	m_OutputSearchTargetBlackboardKey()
,	m_CombatTargetScoreThreshold( 0.0f )
,	m_SearchTargetScoreThreshold( 0.0f )
{
}

WBCompEldThinkerTarget::~WBCompEldThinkerTarget()
{
}

/*virtual*/ void WBCompEldThinkerTarget::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( OutputCombatTargetBlackboardKey );
	m_OutputCombatTargetBlackboardKey = ConfigManager::GetInheritedHash( sOutputCombatTargetBlackboardKey, HashedString::NullString, sDefinitionName );

	STATICHASH( OutputSearchTargetBlackboardKey );
	m_OutputSearchTargetBlackboardKey = ConfigManager::GetInheritedHash( sOutputSearchTargetBlackboardKey, HashedString::NullString, sDefinitionName );

	STATICHASH( CombatTargetScoreThreshold );
	m_CombatTargetScoreThreshold = ConfigManager::GetInheritedFloat( sCombatTargetScoreThreshold, 0.0f, sDefinitionName );

	STATICHASH( SearchTargetScoreThreshold );
	m_SearchTargetScoreThreshold = ConfigManager::GetInheritedFloat( sSearchTargetScoreThreshold, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldThinkerTarget::Tick( float DeltaTime )
{
	XTRACE_FUNCTION;

	Unused( DeltaTime );

	WBEntity* const					pEntity		= GetEntity();
	DEVASSERT( pEntity );

	WBCompRodinKnowledge* const		pKnowledge	= GET_WBCOMP( pEntity, RodinKnowledge );
	ASSERT( pKnowledge );

	WBCompRodinBlackboard* const	pBlackboard	= GET_WBCOMP( pEntity, RodinBlackboard );
	ASSERT( pBlackboard );

	WBEntity*	pSelectedCombatTarget		= NULL;
	float		SelectedCombatTargetScore	= 0.0f;

	WBEntity*	pSelectedSearchTarget		= NULL;
	float		SelectedSearchTargetScore	= 0.0f;

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
		float VisionScore	= 1.0f;

		STATIC_HASHED_STRING( HearingCertainty );
		const float HearingCertainty = Knowledge.GetFloat( sHearingCertainty );
		HearingScore *= HearingCertainty;

		Score += HearingScore;

		// Make sure we search for things that have damaged us.
		STATIC_HASHED_STRING( IsDamager );
		if( Knowledge.GetBool( sIsDamager ) )
		{
			// TODO: Configurate
			Score += 1.0f;
		}

		if( Score >= m_SearchTargetScoreThreshold && Score > SelectedSearchTargetScore )
		{
			pSelectedSearchTarget		= pKnowledgeEntity;
			SelectedSearchTargetScore	= Score;
		}

		// Filter out knowledge entities that have never been visible.
		STATIC_HASHED_STRING( LastSeenTime );
		if( Knowledge.GetFloat( sLastSeenTime ) <= 0.0f )
		{
			continue;
		}

		STATIC_HASHED_STRING( VisionCertainty );
		const float VisionCertainty = Knowledge.GetFloat( sVisionCertainty );
		VisionScore *= VisionCertainty;

		Score += VisionScore;

		// Only consider vision score for selecting combat target.
		if( VisionScore >= m_CombatTargetScoreThreshold && VisionScore > SelectedCombatTargetScore )
		{
			pSelectedCombatTarget		= pKnowledgeEntity;
			SelectedCombatTargetScore	= VisionScore;
		}

		if( Score >= m_SearchTargetScoreThreshold && Score > SelectedSearchTargetScore )
		{
			pSelectedSearchTarget		= pKnowledgeEntity;
			SelectedSearchTargetScore	= Score;
		}
	}

	pBlackboard->SetEntity( m_OutputCombatTargetBlackboardKey, pSelectedCombatTarget );
	pBlackboard->SetEntity( m_OutputSearchTargetBlackboardKey, pSelectedSearchTarget );
}