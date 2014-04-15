#ifndef WBCOMPELDITEM_H
#define WBCOMPELDITEM_H

#include "wbeldritchcomponent.h"
#include "simplestring.h"

class Vector;
class Angles;

class WBCompEldItem : public WBEldritchComponent
{
public:
	WBCompEldItem();
	virtual ~WBCompEldItem();

	DEFINE_WBCOMP( EldItem, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	HashedString	GetSlot() const { return m_Slot; }
	void			SetSlot( const HashedString& NewSlot ) { m_Slot = NewSlot; }

	bool			SuppressesDropDupe() const { return m_SuppressDropDupe; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			SpawnDrop() const;
	bool			GetSpawnDropTransform( Vector& OutLocation, Vector& OutImpulse, Angles& OutOrientation ) const;

	HashedString	m_Slot;					// Config/serialized
	SimpleString	m_DropSpawn;
	float			m_DropSpawnImpulse;
	float			m_DropSpawnImpulseZ;
	float			m_DropSpawnOffsetZ;
	float			m_DropSpawnYaw;
	bool			m_SuppressDropDupe;		// Config; suppress drop spawn if item is replaced in inventory by the same type of item
};

#endif // WBCOMPELDITEM_H