#ifndef RODINBTNODESELECTOR_H
#define RODINBTNODESELECTOR_H

#include "rodinbtnodecompositesingular.h"

class RodinBTNodeSelector : public RodinBTNodeCompositeSingular
{
public:
	RodinBTNodeSelector();
	virtual ~RodinBTNodeSelector();

	DEFINE_RODINBTNODE_FACTORY( Selector );

	virtual ETickStatus Tick( float DeltaTime );
};

#endif // RODINBTNODESELECTOR_H