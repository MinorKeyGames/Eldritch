#ifndef COLLISIONMESH_H
#define COLLISIONMESH_H

class Segment;
class Vector;
class CollisionTriangle;
class CollisionInfo;
class Ray;
class Sphere;

// Collision mesh flags (or layers)
// NOTE: This could be generalized for all collisions (including trace-entity
// and entity-entity), but that would be a lot more code.
#define CMF_Default		0x00000001
#define CMF_Characters	0x00000002
#define CMF_All			0xffffffff

class CollisionMesh
{
public:
	CollisionMesh();
	CollisionMesh( uint NumTris, CollisionTriangle* Tris, uint Flags );
	~CollisionMesh();

	bool	Intersects( const Segment& s, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Ray& r, CollisionInfo* const pInfo = NULL ) const;
	bool	Intersects( const Sphere& s, CollisionInfo* const pInfo = NULL ) const;

	uint				m_NumTris;
	CollisionTriangle*	m_Tris;
	bool				m_MutableCollidable;
	uint				m_Flags;
};

#endif // COLLISIONMESH_H