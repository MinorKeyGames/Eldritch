#ifndef ADDRODINBTNODEFACTORY

#include "BTNodes/rodinbtnodeeldmoveto.h"
#include "BTNodes/rodinbtnodeeldturntoward.h"
#include "BTNodes/rodinbtnodeeldstopmoving.h"
#include "BTNodes/rodinbtnodeeldlookat.h"
#include "BTNodes/rodinbtnodeeldplayanim.h"
#include "BTNodes/rodinbtnodeeldplaybark.h"

#else

ADDRODINBTNODEFACTORY( EldMoveTo );
ADDRODINBTNODEFACTORY( EldTurnToward );
ADDRODINBTNODEFACTORY( EldStopMoving );
ADDRODINBTNODEFACTORY( EldLookAt );
ADDRODINBTNODEFACTORY( EldPlayAnim );
ADDRODINBTNODEFACTORY( EldPlayBark );

#endif