#ifndef CONFIGVAR_H
#define CONFIGVAR_H

// Turn this on to assert that config vars with unique names don't ever have
// colliding hashes. Using this will cause memory leaks at shutdown from all
// the statically-declared names; that's normal.
#define PARANOID_HASH_CHECK 0

// Turn this on to assert that config vars are the types they are expected to be.
// If the client intends to overload a config var name for multiple types, this
// should be left off because the program may do mismatched Gets intentionally.
#define STRICT_TYPE_CHECK 0

#if PARANOID_HASH_CHECK
#include "simplestring.h"
#endif

#include "hashedstring.h"

class ConfigVar
{
public:
	ConfigVar();

	enum EVarType
	{
		EVT_None,
		EVT_Bool,
		EVT_Int,
		EVT_Float,
		EVT_String,
	};

	EVarType		m_Type;
	union
	{
		bool		m_Bool;
		int			m_Int;
		float		m_Float;
		const char*	m_String;
	};
	HashedString	m_Hash;	// Only valid if type is string

#if PARANOID_HASH_CHECK
	SimpleString	m_Name;
#endif
};

#endif // CONFIGVAR_H