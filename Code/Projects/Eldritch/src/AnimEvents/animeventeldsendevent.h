#ifndef ANIMEVENTELDSENDEVENT_H
#define ANIMEVENTELDSENDEVENT_H

// Sends an event to the mesh's entity.
// Because there's no action stack for anim events, we can't query it the way we would for most events.
// TODO: Completely overhaul anim event system to use WB events. But that's beyond the scope I'm dealing with for now.

#include "animevent.h"
#include "hashedstring.h"
#include "wbparamevaluator.h"

class AnimEventEldSendEvent : public AnimEvent
{
public:
	AnimEventEldSendEvent();
	virtual ~AnimEventEldSendEvent();

	DEFINE_ANIMEVENT( EldSendEvent );

	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );
	virtual void	Call( Mesh* pMesh, Animation* pAnimation );

private:
	struct SNamedParameter
	{
		HashedString		m_Name;
		WBParamEvaluator	m_ValuePE;
	};

	HashedString			m_EventName;
	bool					m_QueueEvent;
	float					m_DispatchDelay;
	Array<SNamedParameter>	m_Parameters;
};

#endif // ANIMEVENTELDSENDEVENT_H