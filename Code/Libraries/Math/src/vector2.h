#ifndef VECTOR2_H
#define VECTOR2_H

class Matrix;
class Vector;
class Vector4;

class Vector2
{
public:
	Vector2();
	Vector2( float _x, float _y );
	Vector2( const Vector2& v );
	Vector2( const Vector& v );
	Vector2( const Vector4& v );

	Vector2 operator*( const Matrix& m ) const;
	Vector2& operator*=( const Matrix& m );

	Vector2& operator=( const Vector2& v );
	Vector2 operator-() const;
	Vector2 operator+( const Vector2& v ) const;
	Vector2 operator-( const Vector2& v ) const;
	Vector2 operator*( const Vector2& v ) const;	// Component-wise multiplication
	Vector2 operator/( const Vector2& v ) const;	// Component-wise division
	Vector2 operator*( float f ) const;
	Vector2 operator/( float f ) const;
	Vector2& operator+=( const Vector2& v );
	Vector2& operator-=( const Vector2& v );
	Vector2& operator*=( const Vector2& v );		// Component-wise multiplication
	Vector2& operator/=( const Vector2& v );		// Component-wise division
	Vector2& operator*=( float f );
	Vector2& operator/=( float f );
	bool operator==( const Vector2& v ) const;
	bool operator!=( const Vector2& v ) const;

	//Vector2	Cross( const Vector2& v ) const;
	float	Dot( const Vector2& v ) const;
	void	Negate();
	void	Normalize();
	void	FastNormalize();
	Vector2	GetNormalized() const;
	Vector2	GetFastNormalized() const;
	float	Length() const;
	float	LengthSquared() const;
	Vector2	ProjectionOnto( const Vector2& v ) const;
	Vector2	LERP( float t, const Vector2& v ) const;
	void	Zero();
	void	Set( float _x, float _y );
	bool	IsZero() const;

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
		};

		struct
		{
			float uv_u;
			float uv_v;
		};
		
		float v[2];
	};
};

Vector2 operator*( float f, const Vector2& v );
Vector2 operator/( float f, const Vector2& v );

#endif