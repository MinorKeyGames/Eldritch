#ifndef QUAT_H
#define QUAT_H

class Vector;
class Matrix;
class Angles;

class Quat
{
public:
	Quat();
	Quat( float _w, float _x, float _y, float _z );
	Quat(const Quat& q);
	Quat( float scalar, const Vector& vector );

	Quat& operator=( const Quat& q );
	Quat operator-() const;
	Quat operator+( const Quat& q ) const;
	Quat operator-( const Quat& q ) const;
	Quat operator*( float s ) const;
	Quat operator/( float s ) const;
	Quat operator*( const Quat& q ) const;
	Quat operator/( const Quat& q ) const;

	Quat& operator+=( const Quat& q );
	Quat& operator-=( const Quat& q );
	Quat& operator*=( float s );
	Quat& operator/=( float s );
	Quat& operator*=( const Quat& q );
	Quat& operator/=( const Quat& q );
	bool operator==( const Quat& q ) const;
	bool operator!=( const Quat& q ) const;

	float	Dot( const Quat& q ) const;
	void	Negate();
	float	Length() const;
	float	LengthSquared() const;
	void	Conjugate();
	void	Invert();
	Quat	GetInverse() const;
	void	Normalize();
	void	FastNormalize();
	Quat	GetNormalized() const;
	Quat	GetFastNormalized() const;
	Quat	SLERP( float t, const Quat& q ) const;
	void	Identity();
	void	GetAxisAngle( Vector& OutAxis, float& OutTheta ) const;
	Vector	GetAxis() const;
	float	GetAngle() const;
	Vector	GetVectorPart() const;
	float	GetScalarPart() const;

	Matrix	ToMatrix() const;
	Angles	ToAngles() const;

	static Quat	CreateRotation( const Vector& Axis, const float Theta );

	float w, x, y, z;
};

Quat operator*( float s, const Quat& q );
Quat operator/( float s, const Quat& q );

#endif