#include "core.h"
#include "vector2.h"
#include "matrix.h"
#include "mathcore.h"
#include "vector.h"
#include "vector4.h"
#include "simplestring.h"

Vector2::Vector2() : x(0.0f), y(0.0f) {}
Vector2::Vector2( float _x, float _y ) : x(_x), y(_y) {}
Vector2::Vector2( const Vector2& v ) : x(v.x), y(v.y) {}
Vector2::Vector2( const Vector& v ) : x(v.x), y(v.y) {}

Vector2::Vector2( const Vector4& v )
:	x( v.x )
,	y( v.y )
{
}

Vector2 Vector2::operator*( const Matrix& m ) const
{
	return m * *this;
}

Vector2& Vector2::operator*=( const Matrix& m )
{
	*this = m * *this;
	return *this;
}

Vector2& Vector2::operator=( const Vector2& v )
{
	x = v.x;
	y = v.y;
	return *this;
}

Vector2 Vector2::operator-() const
{
	return Vector2( -x, -y );
}

Vector2 Vector2::operator+( const Vector2& v ) const
{
	return Vector2( x + v.x, y + v.y );
}

Vector2 Vector2::operator-( const Vector2& v ) const
{
	return Vector2( x - v.x, y - v.y );
}

Vector2 Vector2::operator*( const Vector2& v ) const
{
	return Vector2( x * v.x, y * v.y );
}

Vector2 Vector2::operator/( const Vector2& v ) const
{
	return Vector2( x / v.x, y / v.y );
}

Vector2 Vector2::operator*( float f ) const
{
	return Vector2( x * f, y * f );
}

Vector2 Vector2::operator/( float f ) const
{
	float recF = 1.f / f;
	return Vector2( x * recF, y * recF );
}

Vector2& Vector2::operator+=( const Vector2& v )
{
	x += v.x;
	y += v.y;
	return *this;
}

Vector2& Vector2::operator-=( const Vector2& v )
{
	x -= v.x;
	y -= v.y;
	return *this;
}

Vector2& Vector2::operator*=( const Vector2& v )
{
	x *= v.x;
	y *= v.y;
	return *this;
}

Vector2& Vector2::operator/=( const Vector2& v )
{
	x /= v.x;
	y /= v.y;
	return *this;
}

Vector2& Vector2::operator*=( float f )
{
	x *= f;
	y *= f;
	return *this;
}

Vector2& Vector2::operator/=( float f )
{
	float recF = 1.f / f;
	x *= recF;
	y *= recF;
	return *this;
}

// These could stand to be rewritten with consideration
// for very large or very small floating-point numbers
bool Vector2::operator==( const Vector2& v ) const
{
	return ( ( Abs(x - v.x) < EPSILON )
		&& ( Abs(y - v.y) < EPSILON ) );
}

bool Vector2::operator!=( const Vector2& v ) const
{
	return ( ( Abs(x - v.x) >= EPSILON )
		|| ( Abs(y - v.y) >= EPSILON ) );
}

// Maybe return a parallel vector?
//Vector2 Vector2::Cross( const Vector2& v ) const
//{
//}

float Vector2::Dot( const Vector2& v ) const
{
	return ( x * v.x ) + ( y * v.y );
}

void Vector2::Negate()
{
	x = -x;
	y = -y;
}

void Vector2::Normalize()
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return;
	}
	float recLen = 1.f / Len;
	x *= recLen;
	y *= recLen;
}

void Vector2::FastNormalize()
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	x *= recLen;
	y *= recLen;
}

Vector2 Vector2::GetNormalized() const
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return Vector2();
	}
	float recLen = 1.f / Len;
	return Vector2( x * recLen, y * recLen );
}

Vector2 Vector2::GetFastNormalized() const
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	return Vector2( x * recLen, y * recLen );
}

float Vector2::Length() const
{
	return SqRt( ( x * x ) + ( y * y ) );
}

float Vector2::LengthSquared() const
{
	return ( x * x ) + ( y * y );
}

Vector2 Vector2::ProjectionOnto( const Vector2& v ) const
{
	return ( Dot( v ) / v.LengthSquared() ) * v;	// I assume this is still valid for 2D vectors
}

Vector2 Vector2::LERP( float t, const Vector2& v ) const
{
	return *this + ( v - *this ) * t;
}

void Vector2::Zero()
{
	x = 0.f;
	y = 0.f;
}

void Vector2::Set( float _x, float _y )
{
	x = _x;
	y = _y;
}

bool Vector2::IsZero() const
{
	return
		( Abs(x) < EPSILON ) &&
		( Abs(y) < EPSILON );
}

Vector2 operator*( float f, const Vector2& v )
{
	return Vector2( v.x * f, v.y * f );
}

Vector2 operator/( float f, const Vector2& v )
{
	float recF = 1.f / f;
	return Vector2( v.x * recF, v.y * recF );
}

SimpleString Vector2::GetString() const
{
	return SimpleString::PrintF( "(%.02f, %.02f)", x, y );
}

#if BUILD_DEV
bool Vector2::IsValid() const
{
	return
		FIsValid( x ) &&
		FIsValid( y );
}
#endif