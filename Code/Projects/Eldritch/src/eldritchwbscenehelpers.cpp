#include "core.h"
#include "eldritchwbscenehelpers.h"
#include "wbscene.h"
#include "wbentity.h"
#include "Components/wbcompeldtransform.h"
#include "mathcore.h"

#if BUILD_DEBUG
#define BEGIN_ITERATING_ENTITIES do{ m_IteratingEntities = true; } while(0)
#define END_ITERATING_ENTITIES do{ m_IteratingEntities = false; } while(0)
#define CHECK_ITERATING_ENTITIES do{ DEBUGASSERT( !m_IteratingEntities ); } while(0)
#else
#define BEGIN_ITERATING_ENTITIES DoNothing
#define END_ITERATING_ENTITIES DoNothing
#define CHECK_ITERATING_ENTITIES DoNothing
#endif

void WBScene::GetEntitiesByRadius( Array<WBEntity*>& OutEntities, const Vector& Location, const float Radius ) const
{
	const float RadiusSquared = Square( Radius );

	BEGIN_ITERATING_ENTITIES;
	FOR_EACH_MAP( EntityIter, m_Entities, uint, SEntityRef )
	{
		const SEntityRef& EntityRef = EntityIter.GetValue();
		WBEntity* const pEntity = EntityRef.m_Entity;
		DEVASSERT( pEntity );

		if( EntityRef.m_Removed || pEntity->IsDestroyed() )
		{
			continue;
		}

		WBCompEldTransform* const pTransform = pEntity->GetTransformComponent<WBCompEldTransform>();
		if( !pTransform )
		{
			continue;
		}

		const Vector Offset = Location - pTransform->GetLocation();
		if( Offset.LengthSquared() <= RadiusSquared )
		{
			OutEntities.PushBack( pEntity );
		}
	}
	END_ITERATING_ENTITIES;
}