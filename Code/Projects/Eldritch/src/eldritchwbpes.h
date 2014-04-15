#ifndef ADDWBPEFACTORY

#include "PEs/wbpeeldgetlocation.h"
#include "PEs/wbpeeldgetvelocity.h"
#include "PEs/wbpeelddistance.h"
#include "PEs/wbpeeldgetitem.h"
#include "PEs/wbpeeldplayer.h"
#include "PEs/wbpeeldgetfaction.h"
#include "PEs/wbpeeldgetpersistentvar.h"
#include "PEs/wbpeeldgetcharactervo.h"
#include "PEs/wbpeeldgetslot.h"
#include "PEs/wbpeeldhardswitch.h"
#include "PEs/wbpeeldhardscalar.h"

#else

ADDWBPEFACTORY( EldGetLocation );
ADDWBPEFACTORY( EldGetVelocity );
ADDWBPEFACTORY( EldDistance );
ADDWBPEFACTORY( EldGetItem );
ADDWBPEFACTORY( EldPlayer );
ADDWBPEFACTORY( EldGetFaction );
ADDWBPEFACTORY( EldGetPersistentVar );
ADDWBPEFACTORY( EldGetCharacterVO );
ADDWBPEFACTORY( EldGetSlot );
ADDWBPEFACTORY( EldHardSwitch );
ADDWBPEFACTORY( EldHardScalar );

#endif