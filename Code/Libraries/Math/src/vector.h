#ifndef VECTOR_H
#define VECTOR_H

#if BUILD_WINDOWS
#pragma warning( disable: 4201 )	// nonstandard extension used : nameless struct/union
#endif

class Matrix;
class Angles;
class Vector2;
class Vector4;
class Frustum;
class AABB;
class Sphere;
class Line;
class Plane;
class SimpleString;

class Vector
{
public:
	Vector();
	Vector( float _x, float _y, float _z );
	Vector( const Vector& v );
	Vector( const Vector2& v );
	Vector( const Vector4& v );

	Vector operator*( const Matrix& m ) const;
	Vector& operator*=( const Matrix& m );

	Vector& operator=( const Vector& v );
	Vector operator-() const;

	inline Vector operator+( const Vector& v ) const
	{
		return Vector( x + v.x, y + v.y, z + v.z );
	}

	inline Vector operator-( const Vector& v ) const
	{
		return Vector( x - v.x, y - v.y, z - v.z );
	}

	Vector operator*( const Vector& v ) const;	// Component-wise multiplication
	Vector operator/( const Vector& v ) const;	// Component-wise division
	Vector operator*( float f ) const;
	Vector operator/( float f ) const;
	Vector& operator+=( const Vector& v );
	Vector& operator-=( const Vector& v );
	Vector& operator*=( const Vector& v );	// Component-wise multiplication
	Vector& operator/=( const Vector& v );	// Component-wise division
	Vector& operator*=( float f );
	Vector& operator/=( float f );
	bool operator==( const Vector& v ) const;
	bool operator!=( const Vector& v ) const;
	bool operator<( const Vector& v ) const;	// So Vector can be used as a Map key

	bool	Equals( const Vector& v, float Epsilon ) const;
	Vector	Cross( const Vector& w ) const;
	float	Dot( const Vector& w ) const;
	void	Negate();
	void	Normalize();
	void	FastNormalize();
	Vector	GetNormalized() const;
	Vector	GetFastNormalized() const;
	void	GetNormalized( Vector& OutNormalized, float& OutLength, float& OutOneOverLength ) const;
	float	Length() const;

	inline float LengthSquared() const
	{
		return ( x * x ) + ( y * y ) + ( z * z );
	}

	float	LengthSquared2D() const;
	Vector	ProjectionOnto( const Vector& w ) const;
	Vector	LERP( float t, const Vector& w ) const;
	Vector	LERP( const Vector& t, const Vector& w ) const;	// Per-component t values
	void	Zero();
	void	Set( float _x, float _y, float _z );
	bool	IsZero() const;
	bool	AnyZeros() const;
	Vector	GetPerpendicular() const;
	Vector	GetAbs() const;

	Vector	RotateBy( const Angles& Angles ) const;

	Angles	ToAngles() const;	// Produces a zero-roll angle
	uint	ToColor() const;

	bool Intersects( const Frustum& f ) const;
	bool Intersects( const AABB& a ) const;
	bool Intersects( const Sphere& s ) const;
	bool Intersects( const Line& l ) const;
	bool IsOnFrontSide( const Plane& p ) const;
	bool IsOnBackSide( const Plane& p ) const;

	// Returns per-component min/max
	static void	MinMax( const Vector& V1, const Vector& V2, Vector& OutMin, Vector& OutMax );
	static void MinMax( Vector& V1, Vector& V2 );

	Vector	Get2D() const;

	SimpleString	GetString() const;

#if BUILD_DEV
	bool	IsValid() const;
#endif

	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};

		struct
		{
			float r;
			float g;
			float b;
		};

		float v[3];
	};

	static const Vector Up;
};

Vector operator*( float f, const Vector& v );
Vector operator/( float f, const Vector& v );

#endif