#include "core.h"
#include "wbrule.h"
#include "configmanager.h"

WBRule::WBRule()
:	m_Event()
,	m_Conditions()
,	m_Criteria()
{
}

WBRule::~WBRule()
{
}

void WBRule::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Event );
	m_Event = ConfigManager::GetHash( sEvent, HashedString::NullString, sDefinitionName );

#if BUILD_DEV
	if( m_Event == HashedString::NullString )
	{
		PRINTF( "Rule %s has no event.\n", DefinitionName.CStr() );
		WARN;
	}
#endif

	STATICHASH( NumConditions );
	const uint NumConditions = ConfigManager::GetInt( sNumConditions, 0, sDefinitionName );
	for( uint ConditionIndex = 0; ConditionIndex < NumConditions; ++ConditionIndex )
	{
		SCondition& Condition = m_Conditions.PushBack();

		Condition.m_ConditionPE.InitializeFromDefinition( ConfigManager::GetSequenceString( "Condition%d", ConditionIndex, "", sDefinitionName ) );
	}

	STATICHASH( NumCriteria );
	const uint NumCriteria = ConfigManager::GetInt( sNumCriteria, 0, sDefinitionName );
	for( uint CriterionIndex = 0; CriterionIndex < NumCriteria; ++CriterionIndex )
	{
		SCriterion& Criterion = m_Criteria.PushBack();

		Criterion.m_Name = ConfigManager::GetSequenceHash( "Criterion%dName", CriterionIndex, HashedString::NullString, sDefinitionName );
		Criterion.m_MinPE.InitializeFromDefinition( ConfigManager::GetSequenceString( "Criterion%dMin", CriterionIndex, "", sDefinitionName ) );
		Criterion.m_MaxPE.InitializeFromDefinition( ConfigManager::GetSequenceString( "Criterion%dMax", CriterionIndex, "", sDefinitionName ) );

		ASSERT( Criterion.m_Name != HashedString::NullString );
	}
}