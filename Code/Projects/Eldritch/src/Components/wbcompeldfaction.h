#ifndef WBCOMPELDFACTION_H
#define WBCOMPELDFACTION_H

#include "wbeldritchcomponent.h"
#include "eldritchfactions.h"

class WBCompEldFaction : public WBEldritchComponent
{
public:
	WBCompEldFaction();
	virtual ~WBCompEldFaction();

	DEFINE_WBCOMP( EldFaction, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	EldritchFactions::EFactionCon			GetCon( const WBEntity* const pEntityB );
	EldritchFactions::EFactionCon			GetCon( const HashedString& FactionB );
	static EldritchFactions::EFactionCon	GetCon( const WBEntity* const pEntityA, const WBEntity* const pEntityB );

	HashedString	GetFaction() const { return m_Faction; }

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	HashedString	m_Faction;		// Serialized
	bool			m_Immutable;	// Config
};

#endif // WBCOMPELDFACTION_H