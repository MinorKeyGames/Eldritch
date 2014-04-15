#ifndef ELDRITCHPERSISTENCE_H
#define ELDRITCHPERSISTENCE_H

#include "set.h"
#include "hashedstring.h"
#include "wbevent.h"
#include "iwbeventobserver.h"

class IDataStream;

class EldritchPersistence : public IWBEventObserver
{
public:
	EldritchPersistence();
	~EldritchPersistence();

	void			Save( const IDataStream& Stream ) const;
	void			Load( const IDataStream& Stream );

	// IWBEventObserver
	virtual void	HandleEvent( const WBEvent& Event );
	void			RegisterForEvents();

	uint			GetBankMoney() const { return m_BankMoney; }
	void			SetBankMoney( const uint BankMoney ) { m_BankMoney = BankMoney; }

	bool			IsOpenLock( const HashedString& Lock ) const;
	void			AddOpenLock( const HashedString& Lock );

	uint			GetCharacterHeadIndex() const { return m_CharacterHeadIndex; }
	uint			GetCharacterBodyIndex() const { return m_CharacterBodyIndex; }
	void			SetCharacterHeadIndex( const uint HeadIndex ) { m_CharacterHeadIndex = HeadIndex; }
	void			SetCharacterBodyIndex( const uint BodyIndex ) { m_CharacterBodyIndex = BodyIndex; }

	WBEvent&		GetVariableMap() { return m_VariableMap; }

private:
	uint				m_BankMoney;

	Set<HashedString>	m_OpenLocks;

	uint				m_CharacterHeadIndex;
	uint				m_CharacterBodyIndex;

	WBEvent				m_VariableMap;
};

#endif // ELDRITCHPERSISTENCE_H