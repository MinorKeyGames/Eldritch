#ifndef WBCOMPELDVISIBLE_H
#define WBCOMPELDVISIBLE_H

#include "wbeldritchcomponent.h"
#include "vector.h"

class WBCompEldVisible : public WBEldritchComponent
{
public:
	WBCompEldVisible();
	virtual ~WBCompEldVisible();

	DEFINE_WBCOMP( EldVisible, WBEldritchComponent );

	virtual bool	BelongsInComponentArray() { return true; }

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );
	virtual void	AddContextToEvent( WBEvent& Event ) const;

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

	bool			IsVisible() const { return m_Visible; }
	void			SetVisible( const bool Visible ) { m_Visible = Visible; }
	Vector			GetVisibleLocation() const;

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	bool			m_Visible;
};

#endif // WBCOMPELDVISIBLE_H