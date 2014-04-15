#include "core.h"
#include "wbpatternmatching.h"
#include "wbrule.h"
#include "wbevent.h"
#include "wbactionstack.h"

// TODO: Lots of opportunities for optimization here. See the Valve paper.

bool InternalCompare( const WBRule& Rule, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext );

inline bool Matches( const bool Value, const WBRule::SCriterion& Criterion )
{
	return Value == Criterion.m_MinPE.GetBool();
}

inline bool Matches( const int Value, const WBRule::SCriterion& Criterion )
{
	return Value >= Criterion.m_MinPE.GetInt() && Value <= Criterion.m_MaxPE.GetInt();
}

inline bool Matches( const float Value, const WBRule::SCriterion& Criterion )
{
	return Value >= Criterion.m_MinPE.GetFloat() && Value <= Criterion.m_MaxPE.GetFloat();
}

inline bool Matches( const HashedString& Value, const WBRule::SCriterion& Criterion )
{
	return Value == Criterion.m_MinPE.GetString();
}

inline bool Matches( const WBEntity* const Value, const WBRule::SCriterion& Criterion )
{
	return Value == Criterion.m_MinPE.GetEntity();
}

bool WBPatternMatching::SingleCompare( const Array<WBRule>& Rules, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext, uint& OutIndex )
{
	XTRACE_FUNCTION;

	uint BestMatch = 0;
	uint BestMatchScore = 0;

	const uint NumRules = Rules.Size();
	for( uint RuleIndex = 0; RuleIndex < NumRules; ++RuleIndex )
	{
		const WBRule& Rule = Rules[ RuleIndex ];

		// Don't bother testing anything that will be a lesser or equal match to our current best match.
		const uint Score = Rule.GetCriteria().Size() + 1;	// NOTE: Add 1 for the event name, since it's not just a criterion.
		if( Score > BestMatchScore )
		{
			if( Compare( Rule, Event, PEContext ) )
			{
				BestMatch = RuleIndex;
				BestMatchScore = Score;
			}
		}
	}

	OutIndex = BestMatch;
	return ( BestMatchScore > 0 );
}

bool WBPatternMatching::MultiCompare( const Array<WBRule>& Rules, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext, Array<uint>& OutIndices )
{
	XTRACE_FUNCTION;

	OutIndices.Clear();

	const uint NumRules = Rules.Size();
	for( uint RuleIndex = 0; RuleIndex < NumRules; ++RuleIndex )
	{
		const WBRule& Rule = Rules[ RuleIndex ];

		if( Compare( Rule, Event, PEContext ) )
		{
			OutIndices.PushBack( RuleIndex );
		}
	}

	return ( OutIndices.Size() > 0 );
}

bool WBPatternMatching::Compare( const WBRule& Rule, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext )
{
	XTRACE_FUNCTION;

	WBActionStack::Push( Event );
	const bool RetVal = InternalCompare( Rule, Event, PEContext );
	WBActionStack::Pop();

	return RetVal;
}

bool InternalCompare( const WBRule& Rule, const WBEvent& Event, const WBParamEvaluator::SPEContext& PEContext )
{
	XTRACE_FUNCTION;

	if( Rule.GetEvent() != Event.GetEventName() )
	{
		return false;
	}

	const Array<WBRule::SCondition>& Conditions = Rule.GetConditions();
	const uint NumConditions = Conditions.Size();
	for( uint ConditionIndex = 0; ConditionIndex < NumConditions; ++ConditionIndex )
	{
		const WBRule::SCondition& Condition = Conditions[ ConditionIndex ];

		// TODO: Optimization: For Single/MultiCompare, evaluate this once instead of for each event.
		Condition.m_ConditionPE.Evaluate( PEContext );

		if( !Condition.m_ConditionPE.GetBool() )
		{
			return false;
		}
	}

	const Array<WBRule::SCriterion>& Criteria = Rule.GetCriteria();
	const uint NumCriteria = Criteria.Size();
	for( uint CriterionIndex = 0; CriterionIndex < NumCriteria; ++CriterionIndex )
	{
		const WBRule::SCriterion& Criterion = Criteria[ CriterionIndex ];
		const HashedString& CriterionName = Criterion.m_Name;

		// TODO: Optimization: For Single/MultiCompare, evaluate these once instead of for each event.
		Criterion.m_MinPE.Evaluate( PEContext );
		Criterion.m_MaxPE.Evaluate( PEContext );

		bool CriterionMatches = false;

		switch( Criterion.m_MinPE.GetType() )
		{
		case WBParamEvaluator::EPT_None:	{																				break; }
		case WBParamEvaluator::EPT_Bool:	{ CriterionMatches = Matches( Event.GetBool( CriterionName ), Criterion );		break; }
		case WBParamEvaluator::EPT_Int:		{ CriterionMatches = Matches( Event.GetInt( CriterionName ), Criterion );		break; }
		case WBParamEvaluator::EPT_Float:	{ CriterionMatches = Matches( Event.GetFloat( CriterionName ), Criterion );		break; }
		case WBParamEvaluator::EPT_String:	{ CriterionMatches = Matches( Event.GetHash( CriterionName ), Criterion );		break; }
		case WBParamEvaluator::EPT_Entity:	{ CriterionMatches = Matches( Event.GetEntity( CriterionName ), Criterion );	break; }
		case WBParamEvaluator::EPT_Vector:	{ WARNDESC( "Pattern matching on vectors is not currently supported." );		break; }
		default:							{ WARN;																			break; }
		}

		if( !CriterionMatches )
		{
			return false;
		}
	}

	return true;
}