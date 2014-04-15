#ifndef WBCOMPELDINVENTORY_H
#define WBCOMPELDINVENTORY_H

#include "wbeldritchcomponent.h"
#include "map.h"
#include "hashedstring.h"
#include "wbentityref.h"
#include "simplestring.h"

class WBCompEldInventory : public WBEldritchComponent
{
public:
	WBCompEldInventory();
	virtual ~WBCompEldInventory();

	DEFINE_WBCOMP( EldInventory, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	WBEntity*		GetItem( const HashedString& Slot );

	void			AddItem( const SimpleString& DefinitionName );
	void			AddItem( WBEntity* const pItem );
	void			AddItem( const SimpleString& DefinitionName, const HashedString& Slot );
	void			AddItem( WBEntity* const pItem, const HashedString& Slot );
	void			AddItem( const HashedString& Slot, WBEntity* const pItem );
	void			RemoveItem( WBEntity* const pItem );
	void			RemoveItem( const HashedString& Slot );
	void			DropItem( const HashedString& Slot, const bool SuppressSpawn, WBEntity* const pReplacingItem );
	void			RemoveAllItems();
	void			SwapItems( const HashedString& SlotA, const HashedString& SlotB );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			PushPersistence() const;
	void			PullPersistence();

	typedef Map<HashedString, WBEntityRef> TInventoryMap;
	TInventoryMap	m_InventoryMap;	// Serialized, maps categories to inventory in those slots

	Array<SimpleString>	m_InitialItems;	// Config
};

#endif // WBCOMPELDINVENTORY_H