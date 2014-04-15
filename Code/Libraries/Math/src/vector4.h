#ifndef VECTOR4_H
#define VECTOR4_H

class Matrix;
class Vector;
class Vector2;

class Vector4
{
public:
	Vector4();
	Vector4( float _x, float _y, float _z, float _w );
	Vector4( const Vector4& v );
	Vector4( const Vector& v, float _w = 1.0f );
	Vector4( const Vector2& v, float _z = 0.0f, float _w = 0.0f );
	Vector4( uint32 Color );

	Vector4 operator*( const Matrix& m ) const;
	Vector4& operator*=( const Matrix& m );

	Vector4& operator=( const Vector4& v );
	Vector4 operator-() const;
	Vector4 operator+( const Vector4& v ) const;
	Vector4 operator-( const Vector4& v ) const;
	Vector4 operator*( float f ) const;
	Vector4 operator/( float f ) const;
	Vector4& operator+=( const Vector4& v );
	Vector4& operator-=( const Vector4& v );
	Vector4& operator*=( float f );
	Vector4& operator/=( float f );
	bool operator==( const Vector4& v ) const;
	bool operator!=( const Vector4& v ) const;

	//Vector4	Cross( const Vector4& v ) const;	// Maybe not valid (or relevant) in 4D?
	float	Dot( const Vector4& v ) const;
	void	Negate();
	void	Normalize();
	void	FastNormalize();
	Vector4	GetNormalized() const;
	Vector4	GetFastNormalized() const;
	float	Length() const;
	float	LengthSquared() const;
	Vector4	ProjectionOnto( const Vector4& v ) const;
	Vector4	LERP( float t, const Vector4& v ) const;
	void	Zero();
	void	Set( float _x, float _y, float _z, float _w );

	uint	ToColor() const;

	const float*	GetArray() const { return &v[0]; }

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
			float w;
		};

		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		float v[4];
	};
};

Vector4 operator*( float f, const Vector4& v );
Vector4 operator/( float f, const Vector4& v );

#endif