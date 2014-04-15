#include "core.h"
#include "quat.h"
#include "vector.h"
#include "matrix.h"
#include "angles.h"
#include "mathcore.h"

Quat::Quat() : w(1.f), x(0.f), y(0.f), z(0.f) {}
Quat::Quat( float _w, float _x, float _y, float _z ) : w(_w), x(_x), y(_y), z(_z) {}
Quat::Quat( const Quat& q ) : w(q.w), x(q.x), y(q.y), z(q.z) {}
Quat::Quat( float scalar, const Vector& vector ) : w(scalar), x(vector.x), y(vector.y), z(vector.z) {}

Quat& Quat::operator=( const Quat& q )
{
	w = q.w;
	x = q.x;
	y = q.y;
	z = q.z;
	return *this;
}

Quat Quat::operator-() const
{
	return Quat( -w, -x, -y, -z );
}

Quat Quat::operator+( const Quat& q ) const
{
	return Quat( w + q.w, x + q.x, y + q.y, z + q.z );
}

Quat Quat::operator-( const Quat& q ) const
{
	return Quat( w - q.w, x - q.x, y - q.y, z - q.z );
}

Quat Quat::operator*( float s ) const
{
	return Quat( s*w, s*x, s*y, s*z );
}

Quat Quat::operator/( float s ) const
{
	float recS = 1.f / s;
	return Quat( w * recS, x * recS, y * recS, z * recS );
}

// Grassman product
Quat Quat::operator*( const Quat& q ) const
{
	return Quat(
		w * q.w - x * q.x - y * q.y - z * q.z,
		w * q.x + x * q.w + z * q.y - y * q.z,
		w * q.y + y * q.w + x * q.z - z * q.x,
		w * q.z + z * q.w + y * q.x - x * q.y );
}

Quat Quat::operator/( const Quat& q ) const {
	float rd = 1.0f / (q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	return Quat(
		(w * q.w + x * q.x + y * q.y + z * q.z) * rd,
		(x * q.w - w * q.x - z * q.y + y * q.z) * rd,
		(y * q.w + z * q.x - w * q.y - x * q.z) * rd,
		(z * q.w - y * q.x + x * q.y - w * q.z) * rd);
}

Quat& Quat::operator+=( const Quat& q )
{
	w += q.w;
	x += q.x;
	y += q.y;
	z += q.z;
	return *this;
}

Quat& Quat::operator-=( const Quat& q )
{
	w -= q.w;
	x -= q.x;
	y -= q.y;
	z -= q.z;
	return *this;
}

Quat& Quat::operator*=( float s )
{
	w *= s;
	x *= s;
	y *= s;
	z *= s;
	return *this;
}

Quat& Quat::operator/=( float s )
{
	float recS = 1.f / s;
	w *= recS;
	x *= recS;
	y *= recS;
	z *= recS;
	return *this;
}

// Grassman product
Quat& Quat::operator*=( const Quat& q )
{
	*this = *this * q;
	return *this;
}

Quat& Quat::operator/=( const Quat& q )
{
	*this = *this / q;
	return *this;
}

bool Quat::operator==( const Quat& q ) const
{
	return (
		( Abs( w - q.w ) < EPSILON ) &&
		( Abs( x - q.x ) < EPSILON ) &&
		( Abs( y - q.y ) < EPSILON ) &&
		( Abs( z - q.z ) < EPSILON ) );
}

bool Quat::operator!=( const Quat& q ) const
{
	return (
		( Abs( w - q.w ) >= EPSILON ) ||
		( Abs( x - q.x ) >= EPSILON ) ||
		( Abs( y - q.y ) >= EPSILON ) ||
		( Abs( z - q.z ) >= EPSILON ) );
}

float Quat::Dot( const Quat& q ) const
{
	return (w * q.w) + (x * q.x) + (y * q.y) + (z * q.z);
}

void Quat::Negate()
{
	w = -w;
	x = -x;
	y = -y;
	z = -z;
}

float Quat::Length() const
{
	return SqRt( w * w + x * x + y * y + z * z );
}

float Quat::LengthSquared() const
{
	return w * w + x * x + y * y + z * z;
}

void Quat::Conjugate()
{
	x = -x;
	y = -y;
	z = -z;
}

// Reciprocal (multiplicative inverse) of Q = Q* / |Q|^2 (conjugate divided by length-squared)
void Quat::Invert()
{
	float recLenSq = 1.f / ( w * w + x * x + y * y + z * z );
	w *= recLenSq;
	x *= -recLenSq;
	y *= -recLenSq;
	z *= -recLenSq;
}

Quat Quat::GetInverse() const
{
	Quat Inverse( *this );
	Inverse.Invert();
	return Inverse;
}

void Quat::Normalize()
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return;
	}
	float recLen = 1.f / Len;

	// Basic version
	w *= recLen;
	x *= recLen;
	y *= recLen;
	z *= recLen;

	// SSE version
	//__m128 a, b, c;
	//a = _mm_load_ps( &w );
	//b = _mm_load1_ps( &recLen );
	//c = _mm_mul_ps( a, b );
	//_mm_store_ps( &w, c );
}

void Quat::FastNormalize()
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );

	w *= recLen;
	x *= recLen;
	y *= recLen;
	z *= recLen;
}

Quat Quat::GetNormalized() const
{
	float Len = Length();
	if ( Abs( Len ) < EPSILON )
	{
		return Quat();
	}
	float recLen = 1.f / Len;
	return Quat( w * recLen, x * recLen, y * recLen, z * recLen );
}

Quat Quat::GetFastNormalized() const
{
	// No error checking, plus fast inverse square root
	float recLen = FastInvSqRt( LengthSquared() );
	return Quat( w * recLen, x * recLen, y * recLen, z * recLen );
}

Quat Quat::SLERP( float t, const Quat& q ) const
{
	if( *this == q )
	{
		return *this;
	}

	Quat ThisCopy = *this;

	// Calculate angle between quaternions
	float costheta = Clamp( Dot( q ), -1.0f, 1.0f );
	if( costheta < 0.f )
	{
		costheta = -costheta;
		ThisCopy.Negate();
	}
	float theta = ACos( costheta );
	float sinTheta = Sin( theta );
	float ratioA, ratioB;
	if( sinTheta < EPSILON )
	{
		ratioA = 0.5f;
		ratioB = 0.5f;
	}
	else
	{
		float InvSinTheta = 1.0f / sinTheta;
		ratioA = Sin( ( 1.0f - t ) * theta ) * InvSinTheta;
		ratioB = Sin( t * theta ) * InvSinTheta;
	}

	// Calculate interpolated quaternion from the ratios
	return ( ( ThisCopy * ratioA ) + ( q * ratioB ) );
}

void Quat::Identity()
{
	w = 1.f;
	x = 0.f;
	y = 0.f;
	z = 0.f;
}

void Quat::GetAxisAngle( Vector& OutAxis, float& OutTheta ) const
{
	OutTheta = GetAngle();
	OutAxis = GetAxis();
}

Vector Quat::GetAxis() const
{
	float SinTheta = SqRt( 1.0f - w * w );
	if( Abs( SinTheta ) > EPSILON )
	{
		float Recip = 1.0f / SinTheta;
		return Vector( x * Recip, y * Recip, z * Recip );
	}
	else
	{
		return Vector();
	}
}

float Quat::GetAngle() const
{
	return 2.0f * ACos( w );
}

Vector Quat::GetVectorPart() const
{
	return Vector( x, y, z );
}

float Quat::GetScalarPart() const
{
	return w;
}

Matrix Quat::ToMatrix() const
{
	Quat nq = GetNormalized();
	float xx = 2.f * nq.x * nq.x;
	float yy = 2.f * nq.y * nq.y;
	float zz = 2.f * nq.z * nq.z;
	float xy = 2.f * nq.x * nq.y;
	float xz = 2.f * nq.x * nq.z;
	float wx = 2.f * nq.w * nq.x;
	float wz = 2.f * nq.w * nq.z;
	float wy = 2.f * nq.w * nq.y;
	float yz = 2.f * nq.y * nq.z;

	// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
	return Matrix(
		1.f - ( yy + zz ),	xy + wz,			xz - wy,			0.f,
		xy - wz,			1.f - ( xx + zz ),	yz + wx,			0.f,
		xz + wy,			yz - wx,			1.f - ( xx + yy ),	0.f,
		0.f,				0.f,				0.f,				1.f );
}

Angles Quat::ToAngles() const
{
	// This old version wasn't working, so I'm just falling back on always converting through a matrix.
	//const float Pitch = ATan( ( 2.0f * ( w * x + y * z ) ) / ( 1.0f - ( 2.0f * ( x * x + y * y ) ) ) );
	//const float Roll = ASin( 2.0f * ( w * y - x * z ) );
	//const float Yaw = ATan( ( 2.0f * ( w * z + x * y ) ) / ( 1.0f - ( 2.0f * ( y * y + z * z ) ) ) );
	//return Angles( Pitch, Roll, Yaw );

	return ToMatrix().ToAngles();
}

Quat operator*( float s, const Quat& q )
{
	return Quat( s*q.w, s*q.x, s*q.y, s*q.z );
}

Quat operator/( float s, const Quat& q )
{
	float recS = 1.f / s;
	return Quat( q.w * recS, q.x * recS, q.y * recS, q.z * recS );
}

/*static*/ Quat Quat::CreateRotation( const Vector& Axis, const float Theta )
{
	float HalfAngle = Theta * 0.5f;
	Vector v = Axis * Sin( HalfAngle );
	return Quat( Cos( HalfAngle ), v.x, v.y, v.z );
}