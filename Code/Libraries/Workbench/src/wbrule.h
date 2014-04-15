#ifndef WBRULE_H
#define WBRULE_H

#include "array.h"
#include "hashedstring.h"
#include "wbparamevaluator.h"

class WBRule
{
public:
	WBRule();
	~WBRule();

	// Conditions are for arbitrary logic: any PE that returns a bool can be evaluated.
	struct SCondition
	{
		mutable WBParamEvaluator	m_ConditionPE;
	};

	// Criteria are for strict pattern matching: a value in an event between two thresholds.
	struct SCriterion
	{
		HashedString				m_Name;
		mutable WBParamEvaluator	m_MinPE;
		mutable WBParamEvaluator	m_MaxPE;
	};

	void	InitializeFromDefinition( const SimpleString& DefinitionName );

	const HashedString&			GetEvent() const { return m_Event; }
	const Array<SCondition>&	GetConditions() const { return m_Conditions; }
	const Array<SCriterion>&	GetCriteria() const { return m_Criteria; }

private:
	// NOTE: I could just treat the rule/event name as a criterion,
	// but handling it separately is easier than setting up a PE for it.
	HashedString		m_Event;
	Array<SCondition>	m_Conditions;
	Array<SCriterion>	m_Criteria;
};

#endif // WBRULE_H