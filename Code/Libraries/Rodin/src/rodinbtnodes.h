#ifndef ADDRODINBTNODEFACTORY

#include "BTNodes/rodinbtnodeselector.h"
#include "BTNodes/rodinbtnodesequence.h"
#include "BTNodes/rodinbtnodeparallel.h"
#include "BTNodes/rodinbtnodeblackboardwrite.h"
#include "BTNodes/rodinbtnodeconditionpe.h"
#include "BTNodes/rodinbtnodeloop.h"
#include "BTNodes/rodinbtnodelog.h"
#include "BTNodes/rodinbtnodewaitforevent.h"
#include "BTNodes/rodinbtnodeuseresource.h"
#include "BTNodes/rodinbtnodewait.h"
#include "BTNodes/rodinbtnodesendevent.h"
#include "BTNodes/rodinbtnodetimeout.h"
#include "BTNodes/rodinbtnodeusestatmod.h"
#include "BTNodes/rodinbtnodeplayactions.h"
#include "BTNodes/rodinbtnodelookup.h"
#include "BTNodes/rodinbtnodesleep.h"
#include "BTNodes/rodinbtnodecastresult.h"
#include "BTNodes/rodinbtnodenull.h"

#else

ADDRODINBTNODEFACTORY( Selector );
ADDRODINBTNODEFACTORY( Sequence );
ADDRODINBTNODEFACTORY( Parallel );
ADDRODINBTNODEFACTORY( BlackboardWrite );
ADDRODINBTNODEFACTORY( ConditionPE );
ADDRODINBTNODEFACTORY( Loop );
ADDRODINBTNODEFACTORY( Log );
ADDRODINBTNODEFACTORY( WaitForEvent );
ADDRODINBTNODEFACTORY( UseResource );
ADDRODINBTNODEFACTORY( Wait );
ADDRODINBTNODEFACTORY( SendEvent );
ADDRODINBTNODEFACTORY( Timeout );
ADDRODINBTNODEFACTORY( UseStatMod );
ADDRODINBTNODEFACTORY( PlayActions );
ADDRODINBTNODEFACTORY( Lookup );
ADDRODINBTNODEFACTORY( Sleep );
ADDRODINBTNODEFACTORY( CastResult );
ADDRODINBTNODEFACTORY( Null );

#endif