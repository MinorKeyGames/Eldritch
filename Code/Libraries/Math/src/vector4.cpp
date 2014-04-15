#include "core.h"
#include "vector4.h"
#include "vector.h"
#include "vector2.h"
#include "matrix.h"
#include "mathcore.h"
#include "simplestring.h"

Vector4::Vector4() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f ) {}
Vector4::Vector4( float _x, float _y, float _z, float _w ) : x(_x), y(_y), z(_z), w(_w) {}
Vector4::Vector4( const Vector4& v ) : x(v.x), y(v.y), z(v.z), w(v.w) {}

// Note that w will default to 1 when converting a Vector, because
// this is how Vectors are implicitly treated (see matrix-vector multiply)
Vector4::Vector4( const Vector& v, float _w /*= 1.0f*/ )
:	x( v.x )
,	y( v.y )
,	z( v.z )
,	w( _w )
{
}

Vector4::Vector4( const Vector2& v, float _z /*= 0.0f*/, float _w /*= 0.0f*/ )
:	x( v.x )
,	y( v.y )
,	z( _z )
,	w( _w )
{
}

// Ordered RGBA
Vector4::Vector4( uint32 Color )
{
	x = static_cast<float>( R_FROM_COLOR( Color ) ) / 255.0f;
	y = static_cast<float>( G_FROM_COLOR( Color ) ) / 255.0f;
	z = static_cast<float>( B_FROM_COLOR( Color ) ) / 255.0f;
	w = static_cast<float>( A_FROM_COLOR( Color ) ) / 255.0f;
}

Vector4 Vector4::operator*( const Matrix& m ) const
{
	return m * *this;
}

Vector4& Vector4::operator*=( const Matrix& m )
{
	*this = m * *this;
	return *this;
}

Vector4& Vector4::operator=( const Vector4& v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
	return *this;
}

Vector4 Vector4::operator-() const
{
	return Vector4( -x, -y, -z, -w );
}

Vector4 Vector4::operator+( const Vector4& v ) const
{
	return Vector4( x + v.x, y + v.y, z + v.z, w + v.w );
}

Vector4 Vector4::operator-( const Vector4& v ) const
{
	return Vector4( x - v.x, y - v.y, z - v.z, w - v.w );
}

Vector4 Vector4::operator*( float f ) const
{
	return Vector4( x * f, y * f, z * f, w * f );
}

Vector4 Vector4::operator/( float f ) const
{
	float recF = 1.f / f;
	return Vector4( x * recF, y * recF, z * recF, w * recF );
}

Vector4& Vector4::operator+=( const Vector4& v )
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
	return *this;
}

Vector4& Vector4::operator-=( const Vector4& v )
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
	return *this;
}

Vector4& Vector4::operator*=( float f )
{
	x *= f;
	y *= f;
	z *= f;
	w *= f;
	return *this;
}

Vector4& Vector4::operator/=( float f )
{
	float recF = 1.f / f;
	x *= recF;
	y *= recF;
	z *= recF;
	w *= recF;
	return *this;
}

// These could stand to be rewritten with consideration
// for very large or very small floating-point numbers
bool Vector4::operator==( const Vector4& v ) const
{
	return ( ( Abs(x - v.x) < EPSILON )
		&& ( Abs(y - v.y) < EPSILON )
		&& ( Abs(z - v.z) < EPSILON )
		&& ( Abs(w - v.w) < EPSILON ) );
}

bool Vector4::operator!=( const Vector4& v ) const
{
	return ( ( Abs(x - v.x) >= EPSILON )
		|| ( Abs(y - v.y) >= EPSILON )
		|| ( Abs(z - v.z) >= EPSILON )
		|| ( Abs(w - v.w) >= EPSILON ) );
}

//Vector4 Vector4::Cross( const Vector4& v ) const
//{
//	// TODO: This
//	WARN;
//	return Vector4();
//}

float Vector4::Dot(const Vector4& v ) const
{
	return ( x * v.x ) + ( y * v.y ) + ( z * v.z ) + ( w * v.w );
}

void Vector4::Negate()
{
	x = -x;
	y = -y;
	z = -z;
	w = -w;
}

void Vector4::Normalize()
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
	w *= recLen;
}

void Vector4::FastNormalize()
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	x *= recLen;
	y *= recLen;
	z *= recLen;
	w *= recLen;
}

Vector4 Vector4::GetNormalized() const
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return Vector4();
	}
	float recLen = 1.f / Len;
	return Vector4( x * recLen, y * recLen, z * recLen, w * recLen );
}

Vector4 Vector4::GetFastNormalized() const
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	return Vector4( x * recLen, y * recLen, z * recLen, w * recLen );
}

float Vector4::Length() const
{
	return SqRt( ( x * x ) + ( y * y ) + ( z * z ) + ( w * w ) );
}

float Vector4::LengthSquared() const
{
	return ( x * x ) + ( y * y ) + ( z * z ) + ( w * w );
}

Vector4 Vector4::ProjectionOnto( const Vector4& v ) const
{
	return ( Dot(v) / v.LengthSquared() ) * v;	// I assume this is still valid for 4D vectors
}

Vector4 Vector4::LERP( float t, const Vector4& v ) const
{
	return *this + ( v - *this ) * t;
}

void Vector4::Zero()
{
	x = 0.f;
	y = 0.f;
	z = 0.f;
	w = 0.f;
}

void Vector4::Set( float _x, float _y, float _z, float _w )
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

// Assumes RGBA order, mainly because Vector is RGB
uint Vector4::ToColor() const
{
	float R = Saturate( x );
	float G = Saturate( y );
	float B = Saturate( z );
	float A = Saturate( w );

	return ARGB_TO_COLOR( (byte)( A * 255.0f ), (byte)( R * 255.0f ), (byte)( G * 255.0f ), (byte)( B * 255.0f ) );
}

Vector4 operator*( float f, const Vector4& v )
{
	return Vector4( v.x * f, v.y * f, v.z * f, v.w * f );
}

Vector4 operator/( float f, const Vector4& v )
{
	float recF = 1.f / f;
	return Vector4( v.x * recF, v.y * recF, v.z * recF, v.w * recF );
}

SimpleString Vector4::GetString() const
{
	return SimpleString::PrintF( "(%.02f, %.02f, %.02f, %.02f)", x, y, z, w );
}

#if BUILD_DEV
bool Vector4::IsValid() const
{
	return
		FIsValid( x ) &&
		FIsValid( y ) &&
		FIsValid( z ) &&
		FIsValid( w );
}
#endif