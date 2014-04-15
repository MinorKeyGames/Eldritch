#include "core.h"
#include "wbpegetentitybylabel.h"
#include "configmanager.h"
#include "Components/wbcomplabel.h"
#include "wbcomponentarrays.h"

WBPEGetEntityByLabel::WBPEGetEntityByLabel()
:	m_Label()
{
}

WBPEGetEntityByLabel::~WBPEGetEntityByLabel()
{
}

/*virtual*/ void WBPEGetEntityByLabel::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( Label );
	m_Label = ConfigManager::GetHash( sLabel, HashedString::NullString, sDefinitionName );
}

/*virtual*/ void WBPEGetEntityByLabel::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	Unused( Context );

	const Array<WBCompLabel*>* pLabelComponents = WBComponentArrays::GetComponents<WBCompLabel>();
	if( !pLabelComponents )
	{
		return;
	}

	const uint NumLabelComponents = pLabelComponents->Size();
	for( uint LabelComponentIndex = 0; LabelComponentIndex < NumLabelComponents; ++LabelComponentIndex )
	{
		WBCompLabel* const pLabelComponent = ( *pLabelComponents )[ LabelComponentIndex ];
		ASSERT( pLabelComponent );

		if( pLabelComponent->GetLabel() != m_Label )
		{
			continue;
		}

		WBEntity* const pLabelEntity = pLabelComponent->GetEntity();
		ASSERT( pLabelEntity );

		if( pLabelEntity->IsDestroyed() )
		{
			continue;
		}

		EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
		EvaluatedParam.m_Entity	= pLabelComponent->GetEntity();
		return;
	}
}