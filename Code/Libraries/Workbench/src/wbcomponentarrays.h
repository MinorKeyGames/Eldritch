#ifndef WBCOMPONENTARRAYS_H
#define WBCOMPONENTARRAYS_H

#include "array.h"

class WBComponent;
class HashedString;

namespace WBComponentArrays
{
	void						AddComponent( WBComponent* const pComponent );
	void						RemoveComponent( WBComponent* const pComponent );

	const Array<WBComponent*>*	GetComponents( const HashedString& ComponentName );

	template<class C> const Array<C*>*	GetComponents() { return reinterpret_cast<const Array<C*>*>( GetComponents( C::GetStaticName() ) ); }
}

#endif // WBCOMPONENTARRAYS_H