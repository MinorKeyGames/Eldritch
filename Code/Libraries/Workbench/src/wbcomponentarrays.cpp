#include "core.h"
#include "wbcomponentarrays.h"
#include "wbcomponent.h"
#include "map.h"

typedef Array<WBComponent*> TComponentArray;
typedef Map<HashedString, TComponentArray> TComponentArrayMap;
static TComponentArrayMap sComponentArrays;

void WBComponentArrays::AddComponent( WBComponent* const pComponent )
{
	ASSERT( pComponent );

	const HashedString ComponentName = pComponent->GetName();
	TComponentArray& ComponentArray = sComponentArrays[ ComponentName ];

	DEBUGASSERT( !ComponentArray.Find( pComponent ) );	// Make sure this component isn't added twice to the array.
	ComponentArray.PushBack( pComponent );
}

void WBComponentArrays::RemoveComponent( WBComponent* const pComponent )
{
	ASSERT( pComponent );

	const HashedString ComponentName = pComponent->GetName();
	DEBUGASSERT( sComponentArrays.Search( ComponentName ).IsValid() );	// Make sure we're not accidentally creating an array by removing this component.
	TComponentArray& ComponentArray = sComponentArrays[ ComponentName ];

	DEBUGASSERT( ComponentArray.Find( pComponent ) );
	ComponentArray.RemoveItem( pComponent );

	// Clean up after ourselves. If this is the last component of a type to be removed, get rid of its key-value pair.
	// Maps clean up neatly (at time of writing, anyway), so I don't need a global shut down to free memory.
	if( ComponentArray.Size() == 0 )
	{
		sComponentArrays.Remove( ComponentName );
	}
}

const Array<WBComponent*>* WBComponentArrays::GetComponents( const HashedString& ComponentName )
{
	TComponentArrayMap::Iterator ComponentArrayIterator = sComponentArrays.Search( ComponentName );
	if( ComponentArrayIterator.IsValid() )
	{
		return &ComponentArrayIterator.GetValue();
	}
	else
	{
		return NULL;
	}
}