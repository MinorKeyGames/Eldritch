#include "core.h"
#include "wbpeeldgetslot.h"
#include "Components/wbcompelditem.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"
#include "reversehash.h"

WBPEEldGetSlot::WBPEEldGetSlot()
:	m_EntityPE( NULL )
{
}

WBPEEldGetSlot::~WBPEEldGetSlot()
{
	SafeDelete( m_EntityPE );
}

/*virtual*/ void WBPEEldGetSlot::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEEldGetSlot::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam EntityValue;
	m_EntityPE->Evaluate( Context, EntityValue );

	WBEntity* const pEntity = EntityValue.GetEntity();
	if( !pEntity )
	{
		return;
	}

	WBCompEldItem* const pItem = GET_WBCOMP( pEntity, EldItem );
	if( !pItem )
	{
		return;
	}

	const HashedString	SlotHash	= pItem->GetSlot();

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_String;
	EvaluatedParam.m_String	= ReverseHash::ReversedHash( SlotHash );
}