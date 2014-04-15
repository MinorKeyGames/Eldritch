#ifndef KDTREE_H
#define KDTREE_H

#include "plane.h"
#include "aabb.h"

// kd-tree (or specifically, a 3d-tree) used to partition space around static
// world geometry (and maybe later dynamic objects).

class Mesh;
class Ray;
class Segment;
class Ellipsoid;
class CollisionInfo;

// Node as tree
class KDTree
{
public:
	KDTree();
	~KDTree();

	bool	Trace( const Segment& s, CollisionInfo* const pInfo = NULL ) const;
	bool	Trace( const Ray& r, CollisionInfo* const pInfo = NULL ) const;
	bool	SweepAgainst( const Ellipsoid& e, const Vector& v, const AABB& SweepAABB, CollisionInfo* const pInfo = NULL ) const;

	KDTree*	m_FrontChild;
	KDTree*	m_BackChild;

	Plane	m_SplittingPlane;
	AABB	m_FullBound;		// Entire volume of node
	AABB	m_TightBound;		// Tight bound on all meshes in this node and all children (same as m_FullBound for root node)
	AABB	m_LocalBound;		// Tight bound on only meshes in this node (same as m_TightBound for leaf nodes; has no meaning in non-leaf nodes if there are no local meshes)

	uint	m_NumMeshes;
	Mesh**	m_Meshes;
};

#endif // KDTREE_H