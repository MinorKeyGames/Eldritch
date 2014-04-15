#include "core.h"
#include "wbcompreactions.h"
#include "wbpatternmatching.h"
#include "wbaction.h"
#include "configmanager.h"
#include "wbactionfactory.h"
#include "wbrule.h"
#include "wbactionstack.h"

WBCompReactions::WBCompReactions()
:	m_Rules()
,	m_Actions()
,	m_DoMultiCompare( false )
{
}

WBCompReactions::~WBCompReactions()
{
	const uint NumReactions = m_Actions.Size();
	for( uint ReactionsIndex = 0; ReactionsIndex < NumReactions; ++ReactionsIndex )
	{
		TActions& Actions = m_Actions[ ReactionsIndex ];
		const uint NumActions = Actions.Size();
		for( uint ActionsIndex = 0; ActionsIndex < NumActions; ++ActionsIndex )
		{
			SafeDelete( Actions[ ActionsIndex ] );
		}
	}
}

/*virtual*/ void WBCompReactions::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( DoMultiCompare );
	m_DoMultiCompare = ConfigManager::GetInheritedBool( sDoMultiCompare, false, sDefinitionName );

	STATICHASH( NumReactions );
	const uint NumReactions = ConfigManager::GetInheritedInt( sNumReactions, 0, sDefinitionName );

	for( uint ReactionIndex = 0; ReactionIndex < NumReactions; ++ReactionIndex )
	{
		const SimpleString ReactionDef = ConfigManager::GetInheritedSequenceString( "Reaction%d", ReactionIndex, "", sDefinitionName );
		MAKEHASH( ReactionDef );
		ASSERT( ReactionDef != "" );

		STATICHASH( Rule );
		const SimpleString Rule = ConfigManager::GetString( sRule, "", sReactionDef );

#if BUILD_DEV
		if( Rule == "" )
		{
			PRINTF( "Reaction %s has no rule.\n", ReactionDef.CStr() );
			WARN;
		}
#endif

		m_Rules.PushBack().InitializeFromDefinition( Rule );

		TActions& Actions = m_Actions.PushBack();

		WBActionFactory::InitializeActionArray( ReactionDef, Actions );
	}
}

/*virtual*/ void WBCompReactions::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	// Create a context for pattern matching rules to evaluate their PEs.
	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = GetEntity();

	if( m_DoMultiCompare )
	{
		Array<uint> MatchingIndices;
		if( WBPatternMatching::MultiCompare( m_Rules, Event, PEContext, MatchingIndices ) )
		{
			const uint NumMatchingIndices = MatchingIndices.Size();
			for( uint MatchingIndexIndex = 0; MatchingIndexIndex < NumMatchingIndices; ++MatchingIndexIndex )
			{
				const uint MatchingIndex = MatchingIndices[ MatchingIndexIndex ];
				ExecuteActions( m_Actions[ MatchingIndex ], Event );
			}
		}
	}
	else
	{
		uint MatchingIndex;
		if( WBPatternMatching::SingleCompare( m_Rules, Event, PEContext, MatchingIndex ) )
		{
			ExecuteActions( m_Actions[ MatchingIndex ], Event );
		}
	}
}

void WBCompReactions::ExecuteActions( const WBCompReactions::TActions& Actions, const WBEvent& ContextEvent ) const
{
	XTRACE_FUNCTION;

	const uint NumActions = Actions.Size();
	for( uint ActionIndex = 0; ActionIndex < NumActions; ++ActionIndex )
	{
		WBAction* const pAction = Actions[ ActionIndex ];
		DEVASSERT( pAction );

		WBActionStack::Push( ContextEvent );
		pAction->Execute();
		WBActionStack::Pop();
	}
}