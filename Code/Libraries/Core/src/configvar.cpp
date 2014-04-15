#include "core.h"
#include "configvar.h"

ConfigVar::ConfigVar()
:	m_Type( EVT_None )
,	m_Int( 0 )
,	m_Hash( (unsigned long)0 )
#if PARANOID_HASH_CHECK
,	m_Name( "" )
#endif
{
}