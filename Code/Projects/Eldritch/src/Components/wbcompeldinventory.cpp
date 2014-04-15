#include "core.h"
#include "wbcompeldinventory.h"
#include "wbcompelditem.h"
#include "configmanager.h"
#include "idatastream.h"
#include "wbeventmanager.h"
#include "eldritchgame.h"

WBCompEldInventory::WBCompEldInventory()
:	m_InventoryMap()
,	m_InitialItems()
{
}

WBCompEldInventory::~WBCompEldInventory()
{
}

/*virtual*/ void WBCompEldInventory::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( NumInitialItems );
	const uint NumInitialItems = ConfigManager::GetInheritedInt( sNumInitialItems, 0, sDefinitionName );

	for( uint InitialItemIndex = 0; InitialItemIndex < NumInitialItems; ++InitialItemIndex )
	{
		const SimpleString ItemDef = ConfigManager::GetInheritedSequenceString( "InitialItem%d", InitialItemIndex, "", sDefinitionName );
		m_InitialItems.PushBack( ItemDef );
	}
}

WBEntity* WBCompEldInventory::GetItem( const HashedString& Slot )
{
	TInventoryMap::Iterator InventoryIter = m_InventoryMap.Search( Slot );
	return InventoryIter.IsValid() ? InventoryIter.GetValue().Get() : NULL;
}

void WBCompEldInventory::AddItem( const SimpleString& DefinitionName )
{
	WBEntity* const pItem = WBWorld::GetInstance()->CreateEntity( DefinitionName );
	if( pItem )
	{
		AddItem( pItem );
	}
}

void WBCompEldInventory::AddItem( WBEntity* const pItem )
{
	ASSERT( pItem );

	WBCompEldItem* const pItemComponent = GET_WBCOMP( pItem, EldItem );
	ASSERT( pItemComponent );

	AddItem( pItemComponent->GetSlot(), pItem );
}

void WBCompEldInventory::AddItem( const SimpleString& DefinitionName, const HashedString& Slot )
{
	WBEntity* const pItem = WBWorld::GetInstance()->CreateEntity( DefinitionName );
	if( pItem )
	{
		AddItem( pItem, Slot );
	}
}

void WBCompEldInventory::AddItem( WBEntity* const pItem, const HashedString& Slot )
{
	ASSERT( pItem );

	WBCompEldItem* const pItemComponent = GET_WBCOMP( pItem, EldItem );
	ASSERT( pItemComponent );

	pItemComponent->SetSlot( Slot );

	AddItem( Slot, pItem );
}

void WBCompEldInventory::AddItem( const HashedString& Slot, WBEntity* const pItem )
{
	ASSERT( pItem );

	// HACK: If we're adding a weapon, and we already have a weapon, and it's
	// a different weapon or not a "suppress dupe drop" weapon, but the alt
	// weapon slot is free, push current weapon to the alt slot.
	STATIC_HASHED_STRING( Weapon );
	STATIC_HASHED_STRING( WeaponAlt );

	if( Slot == sWeapon )
	{
		WBEntity* const pCurrentWeapon = GetItem( sWeapon );
		WBEntity* const pAltWeapon = GetItem( sWeaponAlt );

		if( pCurrentWeapon && pAltWeapon == NULL )
		{
			WBCompEldItem* const pItemComponent = GET_WBCOMP( pItem, EldItem );
			ASSERT( pItemComponent );

			if( !pItemComponent->SuppressesDropDupe() || pCurrentWeapon->GetName() != pItem->GetName() )
			{
				SwapItems( sWeapon, sWeaponAlt );
			}
		}
	}

	// HACK: If we're adding a weapon, but it is a "suppress dupe drop" weapon
	// and we have a dupe in the alt slot, swap to that slot first.
	bool IsDupeWeapon = false;
	if( Slot == sWeapon )
	{
		WBCompEldItem* const pItemComponent = GET_WBCOMP( pItem, EldItem );
		ASSERT( pItemComponent );
		if( pItemComponent->SuppressesDropDupe() )
		{
			WBEntity* const pAltWeapon = GetItem( sWeaponAlt );
			if( pAltWeapon && pAltWeapon->GetName() == pItem->GetName() )
			{
				IsDupeWeapon = true;
				SwapItems( sWeapon, sWeaponAlt );
			}
		}
	}

	DropItem( Slot, false, pItem );
	m_InventoryMap[ Slot ] = pItem;

	WBEntity* const pEntity = GetEntity();

	// Tell ourselves that the item was equipped (do this *before* the item gets notified; fixes some dependencies)
	WB_MAKE_EVENT( OnItemEquipped, pEntity );
	WB_SET_AUTO( OnItemEquipped, Entity, Item, pItem );
	WB_DISPATCH_EVENT( GetEventManager(), OnItemEquipped, pEntity );

	// Tell the item that it was equipped
	WB_MAKE_EVENT( OnEquipped, pItem );
	WB_LOG_EVENT( OnEquipped );
	WB_SET_AUTO( OnEquipped, Entity, Owner, pEntity );
	WB_DISPATCH_EVENT( GetEventManager(), OnEquipped, pItem );

	// HACK: If we swapped to a dupe weapon, swap back now.
	if( IsDupeWeapon )
	{
		SwapItems( sWeapon, sWeaponAlt );
	}
}

void WBCompEldInventory::RemoveItem( WBEntity* const pItem )
{
	if( !pItem )
	{
		return;
	}

	WBCompEldItem* const pItemComponent = GET_WBCOMP( pItem, EldItem );
	ASSERT( pItemComponent );

	RemoveItem( pItemComponent->GetSlot() );
}

void WBCompEldInventory::RemoveItem( const HashedString& Slot )
{
	DropItem( Slot, true, NULL );
}

void WBCompEldInventory::DropItem( const HashedString& Slot, const bool SuppressSpawn, WBEntity* const pReplacingItem )
{
	WBEntity* const pItem = GetItem( Slot );

	if( pItem )
	{
		WBEntity* const pEntity = GetEntity();

		// NOTE: These events are sent before the item is actually unequipped.
		// There's edge cases either way, don't try to change this.

		// Tell the item that it was unequipped
		WB_MAKE_EVENT( OnUnequipped, pItem );
		WB_LOG_EVENT( OnUnequipped );
		WB_SET_AUTO( OnUnequipped, Entity, Owner, pEntity );
		WB_SET_AUTO( OnUnequipped, Bool, SuppressSpawn, SuppressSpawn );
		WB_SET_AUTO( OnUnequipped, Entity, ReplacingItem, pReplacingItem );
		WB_DISPATCH_EVENT( GetEventManager(), OnUnequipped, pItem );

		// Tell ourselves that the item was unequipped
		WB_MAKE_EVENT( OnItemUnequipped, pEntity );
		WB_SET_AUTO( OnItemUnequipped, Entity, Item, pItem );
		WB_DISPATCH_EVENT( GetEventManager(), OnItemUnequipped, pEntity );

		pItem->Destroy();
	}

	m_InventoryMap.Remove( Slot );

	// HACK: If we dropped our weapon and aren't intended to replace it,
	// and we have an alt weapon, automatically swap to it.
	STATIC_HASHED_STRING( Weapon );
	if( Slot == sWeapon && pReplacingItem == NULL )
	{
		STATIC_HASHED_STRING( WeaponAlt );
		if( GetItem( sWeaponAlt ) != NULL )
		{
			SwapItems( sWeapon, sWeaponAlt );
		}
	}
}

void WBCompEldInventory::RemoveAllItems()
{
	while( m_InventoryMap.Size() )
	{
		RemoveItem( m_InventoryMap.First().GetValue().Get() );
	}
}

void WBCompEldInventory::SwapItems( const HashedString& SlotA, const HashedString& SlotB )
{
	WBEntity* const pItemA = GetItem( SlotA );
	WBEntity* const pItemB = GetItem( SlotB );

	if( pItemA )
	{
		WBCompEldItem* const pItemAItem = GET_WBCOMP( pItemA, EldItem );
		ASSERT( pItemAItem );

		pItemAItem->SetSlot( SlotB );
	}

	if( pItemB )
	{
		WBCompEldItem* const pItemBItem = GET_WBCOMP( pItemB, EldItem );
		ASSERT( pItemBItem );

		pItemBItem->SetSlot( SlotA );
	}

	// Inventory map needs to be kept clean, so remove slots that will be empty now.
	if( pItemB )
	{
		m_InventoryMap[ SlotA ] = pItemB;
	}
	else
	{
		m_InventoryMap.Remove( SlotA );
	}

	if( pItemA )
	{
		m_InventoryMap[ SlotB ] = pItemA;
	}
	else
	{
		m_InventoryMap.Remove( SlotB );
	}

	// Tell ourselves that the items were swapped
	WB_MAKE_EVENT( OnItemsSwapped, GetEntity() );
	WB_SET_AUTO( OnItemsSwapped, Entity, ItemA, pItemA );
	WB_SET_AUTO( OnItemsSwapped, Entity, ItemB, pItemB );
	WB_DISPATCH_EVENT( GetEventManager(), OnItemsSwapped, GetEntity() );
}

/*virtual*/ void WBCompEldInventory::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnSpawned );
	STATIC_HASHED_STRING( RemoveItem );
	STATIC_HASHED_STRING( DropItem );
	STATIC_HASHED_STRING( SwapItems );
	STATIC_HASHED_STRING( PushPersistence );
	STATIC_HASHED_STRING( PullPersistence );

	const HashedString EventName = Event.GetEventName();

	if( EventName == sOnSpawned )
	{
		// Give initial items
		const uint NumInitialItems = m_InitialItems.Size();
		for( uint InitialItemIndex = 0; InitialItemIndex < NumInitialItems; ++InitialItemIndex )
		{
			const SimpleString& ItemDef = m_InitialItems[ InitialItemIndex ];
			AddItem( ItemDef );
		}
	}
	else if( EventName == sRemoveItem )
	{
		STATIC_HASHED_STRING( Slot );
		const HashedString Slot = Event.GetHash( sSlot );

		RemoveItem( Slot );
	}
	else if( EventName == sDropItem )
	{
		STATIC_HASHED_STRING( Slot );
		const HashedString Slot = Event.GetHash( sSlot );

		STATIC_HASHED_STRING( SuppressSpawn );
		const bool SuppressSpawn = Event.GetBool( sSuppressSpawn );

		DropItem( Slot, SuppressSpawn, NULL );
	}
	else if( EventName == sSwapItems )
	{
		STATIC_HASHED_STRING( SlotA );
		const HashedString SlotA = Event.GetHash( sSlotA );

		STATIC_HASHED_STRING( SlotB );
		const HashedString SlotB = Event.GetHash( sSlotB );

		SwapItems( SlotA, SlotB );
	}
	else if( EventName == sPushPersistence )
	{
		PushPersistence();
	}
	else if( EventName == sPullPersistence )
	{
		PullPersistence();
	}
}

void WBCompEldInventory::PushPersistence() const
{
	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	const uint NumInventoryItems = m_InventoryMap.Size();

	STATIC_HASHED_STRING( NumInventoryItems );
	Persistence.SetInt( sNumInventoryItems, NumInventoryItems );

	// Push items in order of their UIDs, so we reconstruct them in the same order they were originally created.
	Array<WBEntityRef> InventoryItems;
	InventoryItems.Reserve( NumInventoryItems );
	FOR_EACH_MAP( ItemIter, m_InventoryMap, HashedString, WBEntityRef )
	{
		ASSERT( ItemIter.GetValue().Get() );
		InventoryItems.PushBack( ItemIter.GetValue() );
	}
	InventoryItems.QuickSort();

	for( uint ItemIndex = 0; ItemIndex < NumInventoryItems; ++ItemIndex )
	{
		WBEntity* const pItem = InventoryItems[ ItemIndex ].Get();
		ASSERT( pItem );

		WBCompEldItem* const pItemComponent = GET_WBCOMP( pItem, EldItem );
		ASSERT( pItemComponent );

		Persistence.SetHash( SimpleString::PrintF( "InventoryItem%d", ItemIndex ), pItem->GetName() );
		Persistence.SetHash( SimpleString::PrintF( "InventorySlot%d", ItemIndex ), pItemComponent->GetSlot() );
	}
}

void WBCompEldInventory::PullPersistence()
{
	RemoveAllItems();

	TPersistence& Persistence = EldritchGame::StaticGetTravelPersistence();

	STATIC_HASHED_STRING( NumInventoryItems );
	const uint NumInventoryItems = Persistence.GetInt( sNumInventoryItems );

	for( uint ItemIndex = 0; ItemIndex < NumInventoryItems; ++ItemIndex )
	{
		const SimpleString InventoryItem = Persistence.GetString( SimpleString::PrintF( "InventoryItem%d", ItemIndex ) );
		const HashedString InventorySlot = Persistence.GetHash( SimpleString::PrintF( "InventorySlot%d", ItemIndex ) );
		AddItem( InventoryItem, InventorySlot );
	}

	// Notify that we've respawned inventory, mainly so character can fix up hand meshes.
	WB_MAKE_EVENT( OnRespawnedInventory, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), OnRespawnedInventory, GetEntity() );
}

#define VERSION_EMPTY			0
#define VERSION_INVENTORYMAP	1
#define VERSION_CURRENT			1

/*virtual*/ uint WBCompEldInventory::GetSerializationSize()
{
	uint Size = 0;

	const uint ItemSize = sizeof( HashedString ) + sizeof( WBEntityRef );

	Size += 4;									// Version
	Size += 4;									// m_InventoryMap.Size()
	Size += ItemSize * m_InventoryMap.Size();	// m_InventoryMap

	return Size;
}

/*virtual*/ void WBCompEldInventory::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteUInt32( m_InventoryMap.Size() );
	FOR_EACH_MAP( InventoryIter, m_InventoryMap, HashedString, WBEntityRef )
	{
		Stream.WriteHashedString( InventoryIter.GetKey() );
		Stream.Write( sizeof( WBEntityRef ), &InventoryIter.GetValue() );
	}
}

/*virtual*/ void WBCompEldInventory::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_INVENTORYMAP )
	{
		ASSERT( m_InventoryMap.Empty() );
		const uint NumItems = Stream.ReadUInt32();
		for( uint ItemIndex = 0; ItemIndex < NumItems; ++ItemIndex )
		{
			const HashedString Slot = Stream.ReadHashedString();
			WBEntityRef Item;
			Stream.Read( sizeof( WBEntityRef ), &Item );
			m_InventoryMap[ Slot ] = Item;
		}
	}
}