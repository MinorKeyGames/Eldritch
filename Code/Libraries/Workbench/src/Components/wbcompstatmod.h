#ifndef WBCOMPSTATMOD_H
#define WBCOMPSTATMOD_H

// NOTE: This currently only modifies floats, because it mostly makes sense to
// modify floats. But I could add int (and maybe bool?) support if I need it.

// TODO: Should stat mods be serialized, or should each client restore its mods?

#include "wbcomponent.h"
#include "multimap.h"
#include "hashedstring.h"

#define WB_MODIFY_FLOAT( name, var, statmod )									\
	STATIC_HASHED_STRING( name );												\
	ASSERT( ( statmod ) );														\
	const float name##AutoVar = ( statmod )->ModifyFloat( ( var ), s##name )

#define WB_MODIFY_FLOAT_SAFE( name, var, statmod )																\
	STATIC_HASHED_STRING( name );																				\
	WBCompStatMod* const name##StatMod = ( statmod );															\
	const float name##Eval = ( var );																			\
	const float name##AutoVar = name##StatMod ? name##StatMod->ModifyFloat( name##Eval, s##name ) : name##Eval

#define WB_MODDED( name ) name##AutoVar

class WBCompStatMod : public WBComponent
{
public:
	WBCompStatMod();
	virtual ~WBCompStatMod();

	DEFINE_WBCOMP( StatMod, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	void			TriggerEvent( const HashedString& Event );
	void			UnTriggerEvent( const HashedString& Event );
	void			SetEventActive( const HashedString& Event, bool Active );

	float			ModifyFloat( const float Value, const HashedString& StatName );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	enum EModifierFunction
	{
		EMF_None,
		EMF_Add,
		EMF_Multiply,
	};

	static EModifierFunction	GetModifierFunctionFromString( const HashedString& Function );

	struct SStatModifier
	{
		SStatModifier();

		bool				m_Active;
		HashedString		m_Event;
		HashedString		m_Stat;
		EModifierFunction	m_Function;
		float				m_Value;
	};

	Multimap<HashedString, SStatModifier>	m_StatModMap;	// Map of stat names to structure, for fastest lookup when modifying value
};

#endif // WBCOMPSTATMOD_H