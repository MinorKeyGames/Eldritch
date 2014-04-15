#ifndef RODINBTNODESEQUENCE_H
#define RODINBTNODESEQUENCE_H

#include "rodinbtnodecompositesingular.h"

class RodinBTNodeSequence : public RodinBTNodeCompositeSingular
{
public:
	RodinBTNodeSequence();
	virtual ~RodinBTNodeSequence();

	DEFINE_RODINBTNODE_FACTORY( Sequence );

	virtual ETickStatus Tick( float DeltaTime );
};

#endif // RODINBTNODESEQUENCE_H