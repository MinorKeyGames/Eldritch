#include "core.h"
#include "wbpeselector.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"

WBPESelector::WBPESelector()
:	m_Selections()
{
}

WBPESelector::~WBPESelector()
{
	FOR_EACH_ARRAY( SelectionIter, m_Selections, SSelection )
	{
		SSelection& Selection = SelectionIter.GetValue();
		SafeDelete( Selection.m_ConditionPE );
		SafeDelete( Selection.m_ValuePE );
	}
}

/*virtual*/ void WBPESelector::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( NumSelections );
	const uint NumSelections = ConfigManager::GetInt( sNumSelections, 0, sDefinitionName );
	for( uint SelectionIndex = 0; SelectionIndex < NumSelections; ++SelectionIndex )
	{
		SSelection& Selection = m_Selections.PushBack();

		const SimpleString Condition = ConfigManager::GetSequenceString( "Selection%dCondition", SelectionIndex, "", sDefinitionName );
		Selection.m_ConditionPE = WBParamEvaluatorFactory::Create( Condition );
		ASSERT( Selection.m_ConditionPE );

		const SimpleString Value = ConfigManager::GetSequenceString( "Selection%dValue", SelectionIndex, "", sDefinitionName );
		Selection.m_ValuePE = WBParamEvaluatorFactory::Create( Value );
		ASSERT( Selection.m_ValuePE );
	}
}

/*virtual*/ void WBPESelector::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	FOR_EACH_ARRAY( SelectionIter, m_Selections, SSelection )
	{
		SSelection& Selection = SelectionIter.GetValue();

		WBParamEvaluator::SEvaluatedParam ConditionValue;
		Selection.m_ConditionPE->Evaluate( Context, ConditionValue );

		if( !ConditionValue.GetBool() )
		{
			continue;
		}

		Selection.m_ValuePE->Evaluate( Context, EvaluatedParam );
		break;
	}
}