#ifndef WBCOMPELDMAPMARKER_H
#define WBCOMPELDMAPMARKER_H

#include "wbeldritchcomponent.h"
#include "simplestring.h"

class WBCompEldMapMarker : public WBEldritchComponent
{
public:
	WBCompEldMapMarker();
	virtual ~WBCompEldMapMarker();

	DEFINE_WBCOMP( EldMapMarker, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			AddMarker();
	void			RemoveMarker();
	void			UpdateMarker();

	bool			m_MarkAsRoom;		// Config. If true, quantize to room center and orient along Y+. If false, use exact transform.
	SimpleString	m_Texture;			// Config

	// TODO:
	// Use this component to control showing/hiding the marker (for
	// compass exits and so forth) and the location/orientation (for
	// the player).
	// Add an option to "uncover" the marker by visiting the room, so
	// exits will remain marked once discovered, with or without a
	// compass.
};

#endif // WBCOMPELDMAPMARKER_H