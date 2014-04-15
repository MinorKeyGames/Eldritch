#ifndef WBCOMPRODINBLACKBOARD_H
#define WBCOMPRODINBLACKBOARD_H

// Since a blackboard is just a map of names to values, I'm just implementing it on top of a WBEvent.
// (Basically just the same as a VariableMap now.)

#include "wbcomponent.h"
#include "wbevent.h"

class WBCompRodinBlackboard : public WBComponent
{
public:
	WBCompRodinBlackboard();
	virtual ~WBCompRodinBlackboard();

	DEFINE_WBCOMP( RodinBlackboard, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	virtual void	Report() const;

	void			SetBool(	const HashedString& Key, bool Value )						{ m_BlackboardEntries.SetBool(		Key, Value ); }
	void			SetInt(		const HashedString& Key, int Value )						{ m_BlackboardEntries.SetInt(		Key, Value ); }
	void			SetFloat(	const HashedString& Key, float Value )						{ m_BlackboardEntries.SetFloat(		Key, Value ); }
	void			SetHash(	const HashedString& Key, const HashedString& Value )		{ m_BlackboardEntries.SetHash(		Key, Value ); }
	void			SetVector(	const HashedString& Key, const Vector& Value )				{ m_BlackboardEntries.SetVector(	Key, Value ); }
	void			SetAngles(	const HashedString& Key, const Angles& Value )				{ m_BlackboardEntries.SetAngles(	Key, Value ); }
	void			SetEntity(	const HashedString& Key, const WBEntityRef& Value )			{ m_BlackboardEntries.SetEntity(	Key, Value ); }

	void			Set(		const HashedString& Key, const WBParamEvaluator& Value )	{ m_BlackboardEntries.Set(			Key, Value ); }

	bool			GetBool(	const HashedString& Key ) const { return m_BlackboardEntries.GetBool(	Key ); }
	int				GetInt(		const HashedString& Key ) const { return m_BlackboardEntries.GetInt(	Key ); }
	float			GetFloat(	const HashedString& Key ) const { return m_BlackboardEntries.GetFloat(	Key ); }
	HashedString	GetHash(	const HashedString& Key ) const { return m_BlackboardEntries.GetHash(	Key ); }
	Vector			GetVector(	const HashedString& Key ) const { return m_BlackboardEntries.GetVector(	Key ); }
	Angles			GetAngles(	const HashedString& Key ) const { return m_BlackboardEntries.GetAngles(	Key ); }
	WBEntity*		GetEntity(	const HashedString& Key ) const { return m_BlackboardEntries.GetEntity(	Key ); }

	WBEvent::EType	GetType(	const HashedString& Key ) const { return m_BlackboardEntries.GetType(	Key ); }

private:
	WBEvent	m_BlackboardEntries;	// Serialized
};

#endif // WBCOMPRODINBLACKBOARD_H