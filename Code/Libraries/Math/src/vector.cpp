#include "core.h"
#include "vector.h"
#include "vector2.h"
#include "vector4.h"
#include "matrix.h"
#include "angles.h"
#include "frustum.h"
#include "aabb.h"
#include "sphere.h"
#include "line.h"
#include "plane.h"
#include "mathcore.h"
#include "simplestring.h"

const Vector Vector::Up( 0.0f, 0.0f, 1.0f );

Vector::Vector() : x(0.0f), y(0.0f), z(0.0f) {}
Vector::Vector( float _x, float _y, float _z ) : x(_x), y(_y), z(_z) {}
Vector::Vector( const Vector& v ) : x(v.x), y(v.y), z(v.z) {}

Vector::Vector( const Vector2& v )
:	x( v.x )
,	y( v.y )
,	z( 0.0f )
{
}

Vector::Vector( const Vector4& v ) : x(v.x), y(v.y), z(v.z) {}

Vector Vector::operator*( const Matrix& m ) const
{
	return m * *this;
}

Vector& Vector::operator*=( const Matrix& m )
{
	*this = m * *this;
	return *this;
}

Vector& Vector::operator=( const Vector& v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vector Vector::operator-() const
{
	return Vector( -x, -y, -z );
}

// Component-wise multiplication
Vector Vector::operator*( const Vector& v ) const
{
	return Vector( x * v.x, y * v.y, z * v.z );
}

// Component-wise division
Vector Vector::operator/( const Vector& v ) const
{
	return Vector( x / v.x, y / v.y, z / v.z );
}

Vector Vector::operator*( float f ) const
{
	return Vector( x * f, y * f, z * f );
}

Vector Vector::operator/( float f ) const
{
	float recF = 1.f / f;
	return Vector( x * recF, y * recF, z * recF );
}

Vector& Vector::operator+=( const Vector& v )
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector& Vector::operator-=( const Vector& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}


// Component-wise multiplication
Vector& Vector::operator*=( const Vector& v )
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

// Component-wise division
Vector& Vector::operator/=( const Vector& v )
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

Vector& Vector::operator*=( float f )
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

Vector& Vector::operator/=( float f )
{
	float recF = 1.f / f;
	x *= recF;
	y *= recF;
	z *= recF;
	return *this;
}

// These could stand to be rewritten with consideration
// for very large or very small floating-point numbers
bool Vector::operator==( const Vector& v ) const
{
	return ( ( Abs(x - v.x) < EPSILON )
		&& ( Abs(y - v.y) < EPSILON )
		&& ( Abs(z - v.z) < EPSILON ) );
}

bool Vector::operator!=( const Vector& v ) const
{
	return ( ( Abs(x - v.x) >= EPSILON )
		|| ( Abs(y - v.y) >= EPSILON )
		|| ( Abs(z - v.z) >= EPSILON ) );
}

uint HashVector( const Vector& v )
{
	uint x = *( uint* )( &v.x );
	uint y = *( uint* )( &v.y );
	uint z = *( uint* )( &v.z );
	x = ( x << 10 ) | ( x >> 22 );
	y = ( y << 20 ) | ( y >> 12 );
	z = ( z << 30 ) | ( z >> 2 );
	return ( x ^ y ) ^ z;
}

float FloatHash( const Vector& v )
{
#define SHIFT 1000.0f
	return v.x + ( v.y * SHIFT ) + ( v.z * SHIFT * SHIFT );
#undef SHIFT
}

// So Vector can be used as a Map key; doesn't have to actually be
// meaningful as long as it works (though I don't think this does)
bool Vector::operator<( const Vector& v ) const
{
	//return ( x < v.x );
	//return HashVector( *this ) < HashVector( v );
	return FloatHash( *this ) < FloatHash( v );
}

bool Vector::Equals( const Vector& v, float Epsilon ) const
{
	return
		( Abs( x - v.x ) <= Epsilon ) &&
		( Abs( y - v.y ) <= Epsilon ) &&
		( Abs( z - v.z ) <= Epsilon );
}

Vector Vector::Cross( const Vector& w ) const
{
	return Vector(
		( y * w.z ) - ( w.y * z ),
		( z * w.x ) - ( w.z * x ),
		( x * w.y ) - ( w.x * y ) );
}

float Vector::Dot(const Vector& w) const
{
	return ( x * w.x ) + ( y * w.y ) + ( z * w.z );
}

void Vector::Negate()
{
	x = -x;
	y = -y;
	z = -z;
}

void Vector::Normalize()
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return;
	}
	float recLen = 1.f / Len;
	x *= recLen;
	y *= recLen;
	z *= recLen;
}

void Vector::FastNormalize()
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	x *= recLen;
	y *= recLen;
	z *= recLen;
}

Vector Vector::GetNormalized() const
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return Vector();
	}
	float recLen = 1.f / Len;
	return Vector( x * recLen, y * recLen, z * recLen );
}

Vector Vector::GetFastNormalized() const
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	return Vector( x * recLen, y * recLen, z * recLen );
}

void Vector::GetNormalized( Vector& OutNormalized, float& OutLength, float& OutOneOverLength ) const
{
	OutLength = Length();
	if ( Abs( OutLength ) < EPSILON )
	{
		OutLength = 0.0f;
		OutOneOverLength = FLT_MAX;
		OutNormalized = Vector();
	}
	else
	{
		OutOneOverLength = 1.0f / OutLength;
		OutNormalized = Vector( x * OutOneOverLength, y * OutOneOverLength, z * OutOneOverLength );
	}
}

float Vector::Length() const
{
	return SqRt( ( x * x ) + ( y * y ) + ( z * z ) );
}

float Vector::LengthSquared2D() const
{
	return ( x * x ) + ( y * y );
}

Vector Vector::ProjectionOnto( const Vector& w ) const
{
	return ( Dot(w) / w.LengthSquared() ) * w;
}

Vector Vector::LERP(float t, const Vector& w) const
{
	return *this + ( w - *this ) * t;
}

Vector Vector::LERP( const Vector& t, const Vector& w ) const
{
	return Vector(
		Lerp( x, w.x, t.x ),
		Lerp( y, w.y, t.y ),
		Lerp( z, w.z, t.z ) );
}

void Vector::Zero()
{
	x = 0.f;
	y = 0.f;
	z = 0.f;
}

void Vector::Set( float _x, float _y, float _z )
{
	x = _x;
	y = _y;
	z = _z;
}

bool Vector::IsZero() const
{
	return
		( Abs(x) < EPSILON ) &&
		( Abs(y) < EPSILON ) &&
		( Abs(z) < EPSILON );
}

bool Vector::AnyZeros() const
{
	return
		( Abs(x) < EPSILON ) ||
		( Abs(y) < EPSILON ) ||
		( Abs(z) < EPSILON );
}

Vector Vector::GetPerpendicular() const
{
	// Cross by the cardinal axis most orthogonal to this vector

	const Vector AbsVector = GetAbs();

	if( AbsVector.x < AbsVector.y && AbsVector.x < AbsVector.z )
	{
		return Cross( Vector( 1.0f, 0.0f, 0.0f ) );
	}
	else if( AbsVector.y < AbsVector.x && AbsVector.y < AbsVector.z )
	{
		return Cross( Vector( 0.0f, 1.0f, 0.0f ) );
	}
	else
	{
		return Cross( Up );
	}
}

Vector Vector::GetAbs() const
{
	return Vector( Abs( x ), Abs( y ), Abs( z ) );
}

Vector operator*( float f, const Vector& v )
{
	return Vector( v.x * f, v.y * f, v.z * f );
}

Vector operator/( float f, const Vector& v )
{
	return Vector( f / v.x, f / v.y, f / v.z );
}

// Could this be faster than two trig ops and a sqrt?
// Answer: NO! I think...
Angles Vector::ToAngles() const
{
	Vector Normal = GetNormalized();
	float Yaw = ATan2( -Normal.x, Normal.y );	// Convention says atan2(y/x), but I'm considering Y+ forward, so a zero yaw should be this way
	float Pitch = ASin( Normal.z );
	return Angles( Pitch, 0.f, Yaw );
}

uint Vector::ToColor() const
{
	float R = Saturate( x );
	float G = Saturate( y );
	float B = Saturate( z );

	return ARGB_TO_COLOR( 255, (byte)( R * 255.0f ), (byte)( G * 255.0f ), (byte)( B * 255.0f ) );
}

bool Vector::Intersects( const Frustum& f ) const
{
	return f.Intersects( *this );
}

bool Vector::Intersects( const AABB& a ) const
{
	return a.Intersects( *this );
}

bool Vector::Intersects( const Sphere& s ) const
{
	return s.Intersects( *this );
}

bool Vector::Intersects( const Line& l ) const
{
	return l.Intersects( *this );
}

bool Vector::IsOnFrontSide( const Plane& p ) const
{
	return p.OnFrontSide( *this );
}

bool Vector::IsOnBackSide( const Plane& p ) const
{
	return p.OnBackSide( *this );
}

/*static*/ void Vector::MinMax( const Vector& V1, const Vector& V2, Vector& OutMin, Vector& OutMax )
{
	::MinMax( V1.x, V2.x, OutMin.x, OutMax.x );
	::MinMax( V1.y, V2.y, OutMin.y, OutMax.y );
	::MinMax( V1.z, V2.z, OutMin.z, OutMax.z );
}

/*static*/ void Vector::MinMax( Vector& V1, Vector& V2 )
{
	::MinMax( V1.x, V2.x );
	::MinMax( V1.y, V2.y );
	::MinMax( V1.z, V2.z );
}

Vector Vector::RotateBy( const Angles& Angles ) const
{
	return ( *this ) * Angles.ToMatrix();
}

Vector Vector::Get2D() const
{
	return Vector( x, y, 0.0f );
}

SimpleString Vector::GetString() const
{
	return SimpleString::PrintF( "(%.02f, %.02f, %.02f)", x, y, z );
}

#if BUILD_DEV
bool Vector::IsValid() const
{
	return
		FIsValid( x ) &&
		FIsValid( y ) &&
		FIsValid( z );
}
#endif