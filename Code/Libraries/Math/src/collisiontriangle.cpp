#include "core.h"
#include "collisiontriangle.h"

CollisionTriangle::CollisionTriangle()
:	m_Triangle()
,	m_AABB()
{
}

CollisionTriangle::CollisionTriangle( const Triangle& t )
:	m_Triangle( t )
,	m_AABB( t.GetAABB() )
{
}

void CollisionTriangle::UpdateAABB()
{
	m_AABB = m_Triangle.GetAABB();
}