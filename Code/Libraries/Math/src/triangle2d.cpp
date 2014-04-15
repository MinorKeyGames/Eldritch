#include "core.h"
#include "triangle2d.h"
#include "mathcore.h"

Triangle2D::Triangle2D()
:	m_Vec1()
,	m_Vec2()
,	m_Vec3()
{
}

Triangle2D::Triangle2D( const Vector2& Vec1, const Vector2& Vec2, const Vector2& Vec3 )
:	m_Vec1( Vec1 )
,	m_Vec2( Vec2 )
,	m_Vec3( Vec3 )
{
}

Box2D Triangle2D::GetBound() const
{
	Box2D TriangleBox( m_Vec1, m_Vec1 );

	TriangleBox.ExpandTo( m_Vec2 );
	TriangleBox.ExpandTo( m_Vec3 );

	return TriangleBox;
}