#include "core.h"
#include "wbactionselector.h"
#include "wbactionfactory.h"
#include "configmanager.h"

WBActionSelector::WBActionSelector()
:	m_Selections()
{
}

WBActionSelector::~WBActionSelector()
{
	FOR_EACH_ARRAY( SelectionIter, m_Selections, SSelection )
	{
		SSelection& Selection = SelectionIter.GetValue();
		SafeDelete( Selection.m_Action );
	}
}

/*virtual*/ void WBActionSelector::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( NumSelections );
	const uint NumSelections = ConfigManager::GetInt( sNumSelections, 0, sDefinitionName );
	for( uint SelectionIndex = 0; SelectionIndex < NumSelections; ++SelectionIndex )
	{
		SSelection& Selection = m_Selections.PushBack();

		const SimpleString Condition = ConfigManager::GetSequenceString( "Selection%dCondition", SelectionIndex, "", sDefinitionName );
		Selection.m_ConditionPE.InitializeFromDefinition( Condition );

		const SimpleString Action = ConfigManager::GetSequenceString( "Selection%dAction", SelectionIndex, "", DefinitionName );
		Selection.m_Action = WBActionFactory::Create( Action );
	}
}

/*virtual*/ void WBActionSelector::Execute()
{
	WBAction::Execute();

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = GetEntity();

	FOR_EACH_ARRAY( SelectionIter, m_Selections, SSelection )
	{
		SSelection& Selection = SelectionIter.GetValue();

		if( !Selection.m_Action )
		{
			continue;
		}

		Selection.m_ConditionPE.Evaluate( PEContext );

		if( !Selection.m_ConditionPE.GetBool() )
		{
			continue;
		}

		Selection.m_Action->Execute();
		break;
	}
}