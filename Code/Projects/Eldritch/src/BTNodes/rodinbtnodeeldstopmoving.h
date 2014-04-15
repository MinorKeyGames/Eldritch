#ifndef RODINBTNODEELDSTOPMOVING_H
#define RODINBTNODEELDSTOPMOVING_H

#include "rodinbtnode.h"

class RodinBTNodeEldStopMoving : public RodinBTNode
{
public:
	RodinBTNodeEldStopMoving();
	virtual ~RodinBTNodeEldStopMoving();

	DEFINE_RODINBTNODE_FACTORY( EldStopMoving );

	virtual ETickStatus	Tick( float DeltaTime );
};

#endif // RODINBTNODEELDSTOPMOVING_H