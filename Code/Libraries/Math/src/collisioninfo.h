#ifndef COLLISIONINFO_H
#define COLLISIONINFO_H

#include "vector.h"
#include "plane.h"
#include "triangle.h"
#include "hashedstring.h"

// These are not Math-level classes; kinda breaks the
// layering to have them here, but it's easier this way.
class Mesh;

class CollisionInfo
{
public:
	CollisionInfo();

	void	ResetInParameters();
	void	ResetOutParameters();

	// Partial assignment
	void	CopyInParametersFrom( const CollisionInfo& OtherInfo );
	void	CopyOutParametersFrom( const CollisionInfo& OtherInfo );

	// In variables
	// Things like collision masks could go here
	bool			m_CollideWorld;
	bool			m_CollideEntities;
	bool			m_CollideNonBlockingEntities;	// Used for ray/segment traces against usable but non-blockable entities
	bool			m_UseCompositeCollision;		// Only for ray/segment traces currently
	bool			m_StopAtAnyCollision;			// Not supported in most projects
	void*			m_CollidingEntity;
	void*			m_TargetEntity;					// If non-null, this is the only entity which will be traced against
	uint			m_CollisionMeshFlags;			// Defined in collisionmesh.h
	uint			m_UserFlags;					// Defined for any given project

	// Out variables
	bool			m_Collision;
	Plane			m_Plane;
	Vector			m_Intersection;	// Point of intersection
	float			m_HitT;			// Distance (or ratio) along a ray or segment
	void*			m_HitEntity;
	Mesh*			m_HitMesh;
	HashedString	m_HitName;		// Description of the hit region. Real strings are too slow to use here.
};

#endif // COLLISIONINFO_H