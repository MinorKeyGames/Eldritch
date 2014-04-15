#include "core.h"
#include "angles.h"
#include "vector.h"
#include "matrix.h"
#include "quat.h"
#include "mathcore.h"
#include "simplestring.h"

Angles::Angles() : Pitch( 0.f ), Roll( 0.f ), Yaw( 0.f ) {}

Angles::Angles( float _Pitch, float _Roll, float _Yaw ) : Pitch( _Pitch ), Roll( _Roll ), Yaw( _Yaw ) {}

Angles& Angles::operator=( const Angles& a )
{
	Pitch = a.Pitch;
	Roll = a.Roll;
	Yaw = a.Yaw;
	return *this;
}

Angles Angles::operator-()
{
	return Angles( -Pitch, -Roll, -Yaw );
}

Angles Angles::operator+( const Angles& a ) const
{
	return Angles( Pitch + a.Pitch, Roll + a.Roll, Yaw + a.Yaw );
}

Angles Angles::operator-( const Angles& a ) const
{
	return Angles( Pitch - a.Pitch, Roll - a.Roll, Yaw - a.Yaw );
}

Angles Angles::operator*( float f ) const
{
	return Angles( Pitch * f, Roll * f, Yaw * f );
}

Angles Angles::operator/( float f ) const
{
	float recF = 1.f / f;
	return Angles( Pitch * recF, Roll * recF, Yaw * recF );
}

Angles& Angles::operator+=( const Angles& a )
{
	Pitch += a.Pitch;
	Roll += a.Roll;
	Yaw += a.Yaw;
	return *this;
}

Angles& Angles::operator-=( const Angles& a )
{
	Pitch -= a.Pitch;
	Roll -= a.Roll;
	Yaw -= a.Yaw;
	return *this;
}

Angles& Angles::operator*=( float f )
{
	Pitch *= f;
	Roll *= f;
	Yaw *= f;
	return *this;
}

Angles& Angles::operator/=( float f )
{
	float recF = 1.f / f;
	Pitch *= recF;
	Roll *= recF;
	Yaw *= recF;
	return *this;
}

// These could stand to be rewritten with consideration
// for very large or very small floating-point numbers
bool Angles::operator==( const Angles& a ) const
{
	return ( ( Abs(Pitch - a.Pitch) < EPSILON )
		&& ( Abs(Roll - a.Roll) < EPSILON )
		&& ( Abs(Yaw - a.Yaw) < EPSILON ) );
}

bool Angles::operator!=( const Angles& a ) const
{
	return ( ( Abs(Pitch - a.Pitch) >= EPSILON )
		|| ( Abs(Roll - a.Roll) >= EPSILON )
		|| ( Abs(Yaw - a.Yaw) >= EPSILON ) );
}

void Angles::Zero()
{
	Pitch	= 0.0f;
	Roll	= 0.0f;
	Yaw		= 0.0f;
}

bool Angles::IsZero() const
{
	return
		Abs( Pitch ) < EPSILON &&
		Abs( Roll ) < EPSILON &&
		Abs( Yaw ) < EPSILON;
}

Vector Angles::GetX() const
{
	float sY = Sin( Yaw );
	float cY = Cos( Yaw );
	float sR = Sin( Roll );
	float cR = Cos( Roll );
	return Vector( ( cR * cY ), ( cR * sY ), -sR );
}

Vector Angles::GetY() const
{
	float sY = Sin( Yaw );
	float cY = Cos( Yaw );
	float sP = Sin( Pitch );
	float cP = Cos( Pitch );
	float sR = Sin( Roll );
	float cR = Cos( Roll );
	return Vector( -( cP * sY ) + ( sP * sR * cY ), ( cP * cY ) + ( sP * sR * sY ), ( sP * cR ) );
}

Vector Angles::GetZ() const
{
	float sY = Sin( Yaw );
	float cY = Cos( Yaw );
	float sP = Sin( Pitch );
	float cP = Cos( Pitch );
	float sR = Sin( Roll );
	float cR = Cos( Roll );
	return Vector( ( sP * sY ) + ( cP * sR * cY ), -( sP * cY ) + ( cP * sR * sY ), ( cP * cR ) );
}

// Equivalent to X = GetX(), etc... Prefer this to doing separate
// axis calls to avoid calling sin/cos more than needed
void Angles::GetAxes( Vector& X, Vector& Y, Vector& Z ) const
{
	float sY = Sin( Yaw );
	float cY = Cos( Yaw );
	float sP = Sin( Pitch );
	float cP = Cos( Pitch );
	float sR = Sin( Roll );
	float cR = Cos( Roll );
	X = Vector( cR*cY, cR*sY, -sR );
	Y = Vector( -cP*sY+sP*sR*cY, cP*cY+sP*sR*sY, sP*cR );
	Z = Vector( sP*sY+cP*sR*cY, -sP*cY+cP*sR*sY, cP*cR );
}

Vector Angles::ToVector() const
{
	return GetY();
}

Vector Angles::ToVector2D() const
{
	float sY = Sin( Yaw );
	float cY = Cos( Yaw );
	return Vector( -sY, cY, 0.0f );
}

Matrix Angles::ToMatrix() const
{
	float cx = Cos( Pitch );
	float sx = Sin( Pitch );
	float cy = Cos( Roll );
	float sy = Sin( Roll );
	float cz = Cos( Yaw );
	float sz = Sin( Yaw );
	return Matrix(
		cy*cz,				cy*sz,				-sy,	0,
		-cx*sz+sx*sy*cz,	cx*cz+sx*sy*sz,		sx*cy,	0,
		sx*sz+cx*sy*cz,		-sx*cz+cx*sy*sz,	cx*cy,	0,
		0,					0,					0,		1.f );
}

Quat Angles::ToQuaternion() const
{
	float cx = Cos( Pitch * 0.5f );
	float sx = Sin( Pitch * 0.5f );
	float cy = Cos( Roll * 0.5f );
	float sy = Sin( Roll * 0.5f );
	float cz = Cos( Yaw * 0.5f );
	float sz = Sin( Yaw * 0.5f );
	return Quat(
		cx * cy * cz + sx * sy * sz,
		sx * cy * cz - cx * sy * sz,
		cx * sy * cz + sx * cy * sz,
		cx * cy * sz - sx * sy * cz );
}

Angles Angles::GetShortestRotation() const
{
	Angles RetVal = ( *this );

	// It seems like this should be able to be expressed in simpler terms,
	// but modding by half these values causes inappropriate wrapping.

	RetVal.Pitch = Mod( RetVal.Pitch, 2.0f * PI );
	if( RetVal.Pitch > PI )
	{
		RetVal.Pitch -= 2.0f * PI;
	}
	else if( RetVal.Pitch < -PI )
	{
		RetVal.Pitch += 2.0f * PI;
	}

	// Initially, I was limiting this to [-PI,PI], but I'm not sure why
	// (and it was never being used, so it never had a chance to fail.)
	RetVal.Roll = Mod( RetVal.Roll, 2.0f * PI );
	if( RetVal.Roll > PI )
	{
		RetVal.Roll -= 2.0f * PI;
	}
	else if( RetVal.Roll < PI )
	{
		RetVal.Roll += 2.0f * PI;
	}

	RetVal.Yaw = Mod( RetVal.Yaw, 2.0f * PI );
	if( RetVal.Yaw > PI )
	{
		RetVal.Yaw -= 2.0f * PI;
	}
	else if( RetVal.Yaw < -PI )
	{
		RetVal.Yaw += 2.0f * PI;
	}

	return RetVal;
}

SimpleString Angles::GetString() const
{
	return SimpleString::PrintF( "(%.02f, %.02f, %.02f)", Pitch, Roll, Yaw );
}