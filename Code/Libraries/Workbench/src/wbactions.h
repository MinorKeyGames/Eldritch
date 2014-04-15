#ifndef ADDWBACTIONFACTORY

#include "Actions/wbactionlog.h"
#include "Actions/wbactiondestroy.h"
#include "Actions/wbactionsendevent.h"
#include "Actions/wbactionunqueueevent.h"
#include "Actions/wbactioncomposite.h"
#include "Actions/wbactionselector.h"
#include "Actions/wbactionsetconfigvar.h"
#include "Actions/wbactionsetvariable.h"
#include "Actions/wbactiontriggerstatmod.h"

#else

ADDWBACTIONFACTORY( Log );
ADDWBACTIONFACTORY( Destroy );
ADDWBACTIONFACTORY( SendEvent );
ADDWBACTIONFACTORY( UnqueueEvent );
ADDWBACTIONFACTORY( Composite );
ADDWBACTIONFACTORY( Selector );
ADDWBACTIONFACTORY( SetConfigVar );
ADDWBACTIONFACTORY( SetVariable );
ADDWBACTIONFACTORY( TriggerStatMod );

#endif