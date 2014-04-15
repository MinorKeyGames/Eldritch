#ifndef WBCOMPREACTIONS_H
#define WBCOMPREACTIONS_H

#include "wbcomponent.h"
#include "array.h"
#include "wbrule.h"
#include "wbaction.h"

class WBCompReactions : public WBComponent
{
public:
	WBCompReactions();
	virtual ~WBCompReactions();

	DEFINE_WBCOMP( Reactions, WBComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	typedef Array<WBAction*> TActions;

	void			ExecuteActions( const TActions& Actions, const WBEvent& ContextEvent ) const;

	// Parallel arrays of rules and the reactions they trigger
	Array<WBRule>	m_Rules;

	Array<TActions>	m_Actions;
	bool			m_DoMultiCompare;
};

#endif // WBCOMPREACTIONS_H