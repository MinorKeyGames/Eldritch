#include "core.h"
#include "wbpeeldgetitem.h"
#include "Components/wbcompeldinventory.h"
#include "configmanager.h"
#include "wbparamevaluatorfactory.h"

WBPEEldGetItem::WBPEEldGetItem()
:	m_EntityPE( NULL )
,	m_SlotPE( NULL )
{
}

WBPEEldGetItem::~WBPEEldGetItem()
{
	SafeDelete( m_EntityPE );
	SafeDelete( m_SlotPE );
}

/*virtual*/ void WBPEEldGetItem::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	MAKEHASH( DefinitionName );

	STATICHASH( EntityPE );
	m_EntityPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sEntityPE, "", sDefinitionName ) );

	STATICHASH( SlotPE );
	m_SlotPE = WBParamEvaluatorFactory::Create( ConfigManager::GetString( sSlotPE, "", sDefinitionName ) );
}

/*virtual*/ void WBPEEldGetItem::Evaluate( const WBParamEvaluator::SPEContext& Context, WBParamEvaluator::SEvaluatedParam& EvaluatedParam ) const
{
	WBParamEvaluator::SEvaluatedParam EntityValue;
	m_EntityPE->Evaluate( Context, EntityValue );

	WBEntity* const pEntity = EntityValue.GetEntity();
	if( !pEntity )
	{
		return;
	}

	WBCompEldInventory* const pInventory = GET_WBCOMP( pEntity, EldInventory );
	if( !pInventory )
	{
		return;
	}

	WBParamEvaluator::SEvaluatedParam SlotValue;
	m_SlotPE->Evaluate( Context, SlotValue );

	ASSERT( SlotValue.m_Type == WBParamEvaluator::EPT_String );

	const HashedString	SlotHash	= SlotValue.m_String;
	WBEntity* const		pItem		= pInventory->GetItem( SlotHash );

	EvaluatedParam.m_Type	= WBParamEvaluator::EPT_Entity;
	EvaluatedParam.m_Entity	= pItem;
}