#ifndef D3D9CONSTANTTABLE_H
#define D3D9CONSTANTTABLE_H

#include "map.h"
#include "hashedstring.h"

class IDataStream;

namespace D3D9ConstantTable
{
	void ParseConstantTable( const IDataStream& Stream, Map<HashedString, uint>& OutTable );
}

#endif