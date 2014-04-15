#include "core.h"
#include "wbactioneldgiveitem.h"
#include "configmanager.h"
#include "wbactionstack.h"
#include "wbevent.h"
#include "Components/wbcompeldinventory.h"

WBActionEldGiveItem::WBActionEldGiveItem()
:	m_ItemDef()
,	m_ItemDefPE()
,	m_GiveToPE()
{
}

WBActionEldGiveItem::~WBActionEldGiveItem()
{
}

/*virtual*/ void WBActionEldGiveItem::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Item );
	m_ItemDef = ConfigManager::GetString( sItem, "", sDefinitionName );

	STATICHASH( ItemPE );
	const SimpleString ItemPE = ConfigManager::GetString( sItemPE, "", sDefinitionName );
	m_ItemDefPE.InitializeFromDefinition( ItemPE );

	STATICHASH( GiveTo );
	const SimpleString GiveToDef = ConfigManager::GetString( sGiveTo, "", sDefinitionName );
	m_GiveToPE.InitializeFromDefinition( GiveToDef );
}

/*virtual*/ void WBActionEldGiveItem::Execute()
{
	WBAction::Execute();

	STATIC_HASHED_STRING( EventOwner );
	WBEntity* const pEntity = WBActionStack::Top().GetEntity( sEventOwner );

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = pEntity;

	m_ItemDefPE.Evaluate( PEContext );
	const SimpleString ItemDef = ( m_ItemDefPE.GetType() == WBParamEvaluator::EPT_String ) ? m_ItemDefPE.GetString() : m_ItemDef;

	m_GiveToPE.Evaluate( PEContext );
	WBEntity* const pGiveToEntity = m_GiveToPE.GetEntity();
	if( pGiveToEntity )
	{
		GiveItemTo( ItemDef, pGiveToEntity );
	}
}

void WBActionEldGiveItem::GiveItemTo( const SimpleString& ItemDef, WBEntity* const pEntity ) const
{
	DEVASSERT( pEntity );

	WBCompEldInventory* const	pInventory		= GET_WBCOMP( pEntity, EldInventory );
	ASSERT( pInventory );

	WBEntity* const				pGivenEntity	= WBWorld::GetInstance()->CreateEntity( ItemDef );
	ASSERT( pGivenEntity );

	pInventory->AddItem( pGivenEntity );
}