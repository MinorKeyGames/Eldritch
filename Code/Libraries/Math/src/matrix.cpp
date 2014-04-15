#include "core.h"
#include "matrix.h"
#include "quat.h"
#include "vector.h"
#include "vector2.h"
#include "vector4.h"
#include "angles.h"
#include "mathcore.h"

Matrix::Matrix()
{
	m[0][0] = 1.f;
	m[0][1] = 0.f;
	m[0][2] = 0.f;
	m[0][3] = 0.f;
	m[1][0] = 0.f;
	m[1][1] = 1.f;
	m[1][2] = 0.f;
	m[1][3] = 0.f;
	m[2][0] = 0.f;
	m[2][1] = 0.f;
	m[2][2] = 1.f;
	m[2][3] = 0.f;
	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = 0.f;
	m[3][3] = 1.f;
}

Matrix::Matrix( float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33 )
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;
	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;
	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;
	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

Matrix::Matrix( float p[4][4] )
{
	m[0][0] = p[0][0];
	m[0][1] = p[0][1];
	m[0][2] = p[0][2];
	m[0][3] = p[0][3];
	m[1][0] = p[1][0];
	m[1][1] = p[1][1];
	m[1][2] = p[1][2];
	m[1][3] = p[1][3];
	m[2][0] = p[2][0];
	m[2][1] = p[2][1];
	m[2][2] = p[2][2];
	m[2][3] = p[2][3];
	m[3][0] = p[3][0];
	m[3][1] = p[3][1];
	m[3][2] = p[3][2];
	m[3][3] = p[3][3];
}

Matrix::Matrix( const Matrix& n )
{
	m[0][0] = n.m[0][0];
	m[0][1] = n.m[0][1];
	m[0][2] = n.m[0][2];
	m[0][3] = n.m[0][3];
	m[1][0] = n.m[1][0];
	m[1][1] = n.m[1][1];
	m[1][2] = n.m[1][2];
	m[1][3] = n.m[1][3];
	m[2][0] = n.m[2][0];
	m[2][1] = n.m[2][1];
	m[2][2] = n.m[2][2];
	m[2][3] = n.m[2][3];
	m[3][0] = n.m[3][0];
	m[3][1] = n.m[3][1];
	m[3][2] = n.m[3][2];
	m[3][3] = n.m[3][3];
}

Matrix& Matrix::operator=( const Matrix& n )
{
	m[0][0] = n.m[0][0];
	m[0][1] = n.m[0][1];
	m[0][2] = n.m[0][2];
	m[0][3] = n.m[0][3];
	m[1][0] = n.m[1][0];
	m[1][1] = n.m[1][1];
	m[1][2] = n.m[1][2];
	m[1][3] = n.m[1][3];
	m[2][0] = n.m[2][0];
	m[2][1] = n.m[2][1];
	m[2][2] = n.m[2][2];
	m[2][3] = n.m[2][3];
	m[3][0] = n.m[3][0];
	m[3][1] = n.m[3][1];
	m[3][2] = n.m[3][2];
	m[3][3] = n.m[3][3];
	return *this;
}

Matrix Matrix::operator+( const Matrix& n ) const
{
	return Matrix(
		m[0][0] + n.m[0][0], m[0][1] + n.m[0][1], m[0][2] + n.m[0][2], m[0][3] + n.m[0][3],
		m[1][0] + n.m[1][0], m[1][1] + n.m[1][1], m[1][2] + n.m[1][2], m[1][3] + n.m[1][3],
		m[2][0] + n.m[2][0], m[2][1] + n.m[2][1], m[2][2] + n.m[2][2], m[2][3] + n.m[2][3],
		m[3][0] + n.m[3][0], m[3][1] + n.m[3][1], m[3][2] + n.m[3][2], m[3][3] + n.m[3][3] );
}

Matrix Matrix::operator-( const Matrix& n ) const
{
	return Matrix(
		m[0][0] - n.m[0][0], m[0][1] - n.m[0][1], m[0][2] - n.m[0][2], m[0][3] - n.m[0][3],
		m[1][0] - n.m[1][0], m[1][1] - n.m[1][1], m[1][2] - n.m[1][2], m[1][3] - n.m[1][3],
		m[2][0] - n.m[2][0], m[2][1] - n.m[2][1], m[2][2] - n.m[2][2], m[2][3] - n.m[2][3],
		m[3][0] - n.m[3][0], m[3][1] - n.m[3][1], m[3][2] - n.m[3][2], m[3][3] - n.m[3][3] );
}

Matrix Matrix::operator*( const Matrix& n ) const
{
	return Matrix(
		m[0][0] * n.m[0][0] + m[0][1] * n.m[1][0] + m[0][2] * n.m[2][0] + m[0][3] * n.m[3][0],
		m[0][0] * n.m[0][1] + m[0][1] * n.m[1][1] + m[0][2] * n.m[2][1] + m[0][3] * n.m[3][1],
		m[0][0] * n.m[0][2] + m[0][1] * n.m[1][2] + m[0][2] * n.m[2][2] + m[0][3] * n.m[3][2],
		m[0][0] * n.m[0][3] + m[0][1] * n.m[1][3] + m[0][2] * n.m[2][3] + m[0][3] * n.m[3][3],
		m[1][0] * n.m[0][0] + m[1][1] * n.m[1][0] + m[1][2] * n.m[2][0] + m[1][3] * n.m[3][0],
		m[1][0] * n.m[0][1] + m[1][1] * n.m[1][1] + m[1][2] * n.m[2][1] + m[1][3] * n.m[3][1],
		m[1][0] * n.m[0][2] + m[1][1] * n.m[1][2] + m[1][2] * n.m[2][2] + m[1][3] * n.m[3][2],
		m[1][0] * n.m[0][3] + m[1][1] * n.m[1][3] + m[1][2] * n.m[2][3] + m[1][3] * n.m[3][3],
		m[2][0] * n.m[0][0] + m[2][1] * n.m[1][0] + m[2][2] * n.m[2][0] + m[2][3] * n.m[3][0],
		m[2][0] * n.m[0][1] + m[2][1] * n.m[1][1] + m[2][2] * n.m[2][1] + m[2][3] * n.m[3][1],
		m[2][0] * n.m[0][2] + m[2][1] * n.m[1][2] + m[2][2] * n.m[2][2] + m[2][3] * n.m[3][2],
		m[2][0] * n.m[0][3] + m[2][1] * n.m[1][3] + m[2][2] * n.m[2][3] + m[2][3] * n.m[3][3],
		m[3][0] * n.m[0][0] + m[3][1] * n.m[1][0] + m[3][2] * n.m[2][0] + m[3][3] * n.m[3][0],
		m[3][0] * n.m[0][1] + m[3][1] * n.m[1][1] + m[3][2] * n.m[2][1] + m[3][3] * n.m[3][1],
		m[3][0] * n.m[0][2] + m[3][1] * n.m[1][2] + m[3][2] * n.m[2][2] + m[3][3] * n.m[3][2],
		m[3][0] * n.m[0][3] + m[3][1] * n.m[1][3] + m[3][2] * n.m[2][3] + m[3][3] * n.m[3][3] );
}

Matrix& Matrix::operator+=( const Matrix& n )
{
	m[0][0] += n.m[0][0];
	m[0][1] += n.m[0][1];
	m[0][2] += n.m[0][2];
	m[0][3] += n.m[0][3];
	m[1][0] += n.m[1][0];
	m[1][1] += n.m[1][1];
	m[1][2] += n.m[1][2];
	m[1][3] += n.m[1][3];
	m[2][0] += n.m[2][0];
	m[2][1] += n.m[2][1];
	m[2][2] += n.m[2][2];
	m[2][3] += n.m[2][3];
	m[3][0] += n.m[3][0];
	m[3][1] += n.m[3][1];
	m[3][2] += n.m[3][2];
	m[3][3] += n.m[3][3];
	return *this;
}

Matrix& Matrix::operator-=( const Matrix& n )
{
	m[0][0] -= n.m[0][0];
	m[0][1] -= n.m[0][1];
	m[0][2] -= n.m[0][2];
	m[0][3] -= n.m[0][3];
	m[1][0] -= n.m[1][0];
	m[1][1] -= n.m[1][1];
	m[1][2] -= n.m[1][2];
	m[1][3] -= n.m[1][3];
	m[2][0] -= n.m[2][0];
	m[2][1] -= n.m[2][1];
	m[2][2] -= n.m[2][2];
	m[2][3] -= n.m[2][3];
	m[3][0] -= n.m[3][0];
	m[3][1] -= n.m[3][1];
	m[3][2] -= n.m[3][2];
	m[3][3] -= n.m[3][3];
	return *this;
}

Matrix& Matrix::operator*=( const Matrix& n )
{
	*this = *this * n;
	return *this;
}

Matrix Matrix::operator*( float f ) const
{
	return Matrix(
		m[0][0] * f, m[0][1] * f, m[0][2] * f, m[0][3] * f,
		m[1][0] * f, m[1][1] * f, m[1][2] * f, m[1][3] * f,
		m[2][0] * f, m[2][1] * f, m[2][2] * f, m[2][3] * f,
		m[3][0] * f, m[3][1] * f, m[3][2] * f, m[3][3] * f );
}

Matrix Matrix::operator/( float f ) const
{
	float recF = 1.f / f;
	return Matrix(
		m[0][0] * recF, m[0][1] * recF, m[0][2] * recF, m[0][3] * recF,
		m[1][0] * recF, m[1][1] * recF, m[1][2] * recF, m[1][3] * recF,
		m[2][0] * recF, m[2][1] * recF, m[2][2] * recF, m[2][3] * recF,
		m[3][0] * recF, m[3][1] * recF, m[3][2] * recF, m[3][3] * recF );
}

Matrix& Matrix::operator*=( float f )
{
	m[0][0] *= f;
	m[0][1] *= f;
	m[0][2] *= f;
	m[0][3] *= f;
	m[1][0] *= f;
	m[1][1] *= f;
	m[1][2] *= f;
	m[1][3] *= f;
	m[2][0] *= f;
	m[2][1] *= f;
	m[2][2] *= f;
	m[2][3] *= f;
	m[3][0] *= f;
	m[3][1] *= f;
	m[3][2] *= f;
	m[3][3] *= f;
	return *this;
}

Matrix& Matrix::operator/=( float f )
{
	float recF = 1.f / f;
	m[0][0] *= recF;
	m[0][1] *= recF;
	m[0][2] *= recF;
	m[0][3] *= recF;
	m[1][0] *= recF;
	m[1][1] *= recF;
	m[1][2] *= recF;
	m[1][3] *= recF;
	m[2][0] *= recF;
	m[2][1] *= recF;
	m[2][2] *= recF;
	m[2][3] *= recF;
	m[3][0] *= recF;
	m[3][1] *= recF;
	m[3][2] *= recF;
	m[3][3] *= recF;
	return *this;
}

Vector Matrix::operator*( const Vector& v ) const
{
	return Vector(
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0],
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1],
		m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] );
}

Vector4 Matrix::operator*( const Vector4& v ) const
{
	return Vector4(
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w,
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w,
		m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w,
		m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w );
}

Vector2 Matrix::operator*( const Vector2& v ) const
{
	return Vector2(
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] + m[3][0] ,
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] + m[3][1] );
}

// These could stand to be rewritten with consideration
// for very large or very small floating-point numbers
bool Matrix::operator==( const Matrix& n ) const
{
	return ( Abs(m[0][0] - n.m[0][0]) < EPSILON &&
		Abs(m[0][1] - n.m[0][1]) < EPSILON &&
		Abs(m[0][2] - n.m[0][2]) < EPSILON &&
		Abs(m[0][3] - n.m[0][3]) < EPSILON &&
		Abs(m[1][0] - n.m[1][0]) < EPSILON &&
		Abs(m[1][1] - n.m[1][1]) < EPSILON &&
		Abs(m[1][2] - n.m[1][2]) < EPSILON &&
		Abs(m[1][3] - n.m[1][3]) < EPSILON &&
		Abs(m[2][0] - n.m[2][0]) < EPSILON &&
		Abs(m[2][1] - n.m[2][1]) < EPSILON &&
		Abs(m[2][2] - n.m[2][2]) < EPSILON &&
		Abs(m[2][3] - n.m[2][3]) < EPSILON &&
		Abs(m[3][0] - n.m[3][0]) < EPSILON &&
		Abs(m[3][1] - n.m[3][1]) < EPSILON &&
		Abs(m[3][2] - n.m[3][2]) < EPSILON &&
		Abs(m[3][3] - n.m[3][3]) < EPSILON );
}

bool Matrix::operator!=( const Matrix& n ) const {
	return ( Abs(m[0][0] - n.m[0][0]) >= EPSILON ||
		Abs(m[0][1] - n.m[0][1]) >= EPSILON ||
		Abs(m[0][2] - n.m[0][2]) >= EPSILON ||
		Abs(m[0][3] - n.m[0][3]) >= EPSILON ||
		Abs(m[1][0] - n.m[1][0]) >= EPSILON ||
		Abs(m[1][1] - n.m[1][1]) >= EPSILON ||
		Abs(m[1][2] - n.m[1][2]) >= EPSILON ||
		Abs(m[1][3] - n.m[1][3]) >= EPSILON ||
		Abs(m[2][0] - n.m[2][0]) >= EPSILON ||
		Abs(m[2][1] - n.m[2][1]) >= EPSILON ||
		Abs(m[2][2] - n.m[2][2]) >= EPSILON ||
		Abs(m[2][3] - n.m[2][3]) >= EPSILON ||
		Abs(m[3][0] - n.m[3][0]) >= EPSILON ||
		Abs(m[3][1] - n.m[3][1]) >= EPSILON ||
		Abs(m[3][2] - n.m[3][2]) >= EPSILON ||
		Abs(m[3][3] - n.m[3][3]) >= EPSILON );
}

void Matrix::Transpose()
{
	float t = m[0][1];
	m[0][1] = m[1][0];
	m[1][0] = t;

	t = m[0][2];
	m[0][2] = m[2][0];
	m[2][0] = t;

	t = m[0][3];
	m[0][3] = m[3][0];
	m[3][0] = t;

	t = m[1][2];
	m[1][2] = m[2][1];
	m[2][1] = t;

	t = m[1][3];
	m[1][3] = m[3][1];
	m[3][1] = t;

	t = m[2][3];
	m[2][3] = m[3][2];
	m[3][2] = t;
}

Matrix Matrix::GetTransposed() const
{
	return Matrix(
		m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3] );
}

float Matrix::Determinant() const
{
	float kplo = ((m[2][2] * m[3][3]) - (m[2][3] * m[3][2]));
	float jpln = ((m[2][1] * m[3][3]) - (m[2][3] * m[3][1]));
	float jokn = ((m[2][1] * m[3][2]) - (m[2][2] * m[3][1]));
	float iplm = ((m[2][0] * m[3][3]) - (m[2][3] * m[3][0]));
	float iokm = ((m[2][0] * m[3][2]) - (m[2][2] * m[3][0]));
	float injm = ((m[2][0] * m[3][1]) - (m[2][1] * m[3][0]));
	float adet = m[0][0] * ((m[1][1] * kplo) - (m[1][2] * jpln) + (m[1][3] * jokn));
	float bdet = m[0][1] * ((m[1][0] * kplo) - (m[1][2] * iplm) + (m[1][3] * iokm));
	float cdet = m[0][2] * ((m[1][0] * jpln) - (m[1][1] * iplm) + (m[1][3] * injm));
	float ddet = m[0][3] * ((m[1][0] * jokn) - (m[1][1] * iokm) + (m[1][2] * injm));

	return ( ( adet - bdet ) + ( cdet - ddet ) );
}

void Matrix::Invert()
{
	float kplo = m[2][2] * m[3][3] - m[2][3] * m[3][2];
	float jpln = m[2][1] * m[3][3] - m[2][3] * m[3][1];
	float jokn = m[2][1] * m[3][2] - m[2][2] * m[3][1];
	float iplm = m[2][0] * m[3][3] - m[2][3] * m[3][0];
	float iokm = m[2][0] * m[3][2] - m[2][2] * m[3][0];
	float injm = m[2][0] * m[3][1] - m[2][1] * m[3][0];

	float gpho = m[1][2] * m[3][3] - m[1][3] * m[3][2];
	float fphn = m[1][1] * m[3][3] - m[1][3] * m[3][1];
	float fogn = m[1][1] * m[3][2] - m[1][2] * m[3][1];
	float ephm = m[1][0] * m[3][3] - m[1][3] * m[3][0];
	float eogm = m[1][0] * m[3][2] - m[1][2] * m[3][0];
	float enfm = m[1][0] * m[3][1] - m[1][1] * m[3][0];

	float glhk = m[1][2] * m[2][3] - m[1][3] * m[2][2];
	float flhj = m[1][1] * m[2][3] - m[1][3] * m[2][1];
	float fkgj = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	float elhi = m[1][0] * m[2][3] - m[1][3] * m[2][0];
	float ekgi = m[1][0] * m[2][2] - m[1][2] * m[2][0];
	float ejfi = m[1][0] * m[2][1] - m[1][1] * m[2][0];

	// Adjoint matrix is the transpose of the cofactor matrix,
	// or matrix of determinants of submatrices
	float adj[4][4] =
	{
		{
			(m[1][1] * kplo - m[1][2] * jpln + m[1][3] * jokn),
			-(m[0][1] * kplo - m[0][2] * jpln + m[0][3] * jokn),
			(m[0][1] * gpho - m[0][2] * fphn + m[0][3] * fogn),
			-(m[0][1] * glhk - m[0][2] * flhj + m[0][3] * fkgj)
		},
		{
			-(m[1][0] * kplo - m[1][2] * iplm + m[1][3] * iokm),
			(m[0][0] * kplo - m[0][2] * iplm + m[0][3] * iokm),
			-(m[0][0] * gpho - m[0][2] * ephm + m[0][3] * eogm),
			(m[0][0] * glhk - m[0][2] * elhi + m[0][3] * ekgi)
		},
		{
			(m[1][0] * jpln - m[1][1] * iplm + m[1][3] * injm),
			-(m[0][0] * jpln - m[0][1] * iplm + m[0][3] * injm),
			(m[0][0] * fphn - m[0][1] * ephm + m[0][3] * enfm),
			-(m[0][0] * flhj - m[0][1] * elhi + m[0][3] * ejfi)
		},
		{	-(m[1][0] * jokn - m[1][1] * iokm + m[1][2] * injm),
			(m[0][0] * jokn - m[0][1] * iokm + m[0][2] * injm),
			-(m[0][0] * fogn - m[0][1] * eogm + m[0][2] * enfm),
			(m[0][0] * fkgj - m[0][1] * ekgi + m[0][2] * ejfi)
		}
	};

	// Reuse the values from the adjoint matrix
	float adet = m[0][0] * adj[0][0];
	float bdet = m[0][1] * adj[1][0];
	float cdet = m[0][2] * adj[2][0];
	float ddet = m[0][3] * adj[3][0];

	float det = adet + bdet + cdet + ddet;
	float recDet = 1.f / det;

	m[0][0] = adj[0][0] * recDet;
	m[0][1] = adj[0][1] * recDet;
	m[0][2] = adj[0][2] * recDet;
	m[0][3] = adj[0][3] * recDet;
	m[1][0] = adj[1][0] * recDet;
	m[1][1] = adj[1][1] * recDet;
	m[1][2] = adj[1][2] * recDet;
	m[1][3] = adj[1][3] * recDet;
	m[2][0] = adj[2][0] * recDet;
	m[2][1] = adj[2][1] * recDet;
	m[2][2] = adj[2][2] * recDet;
	m[2][3] = adj[2][3] * recDet;
	m[3][0] = adj[3][0] * recDet;
	m[3][1] = adj[3][1] * recDet;
	m[3][2] = adj[3][2] * recDet;
	m[3][3] = adj[3][3] * recDet;
}

Matrix Matrix::GetInverse() const
{
	Matrix Inverse( *this );
	Inverse.Invert();
	return Inverse;
}

Matrix Matrix::GetCofactor() const
{
	float kplo = m[2][2] * m[3][3] - m[2][3] * m[3][2];
	float jpln = m[2][1] * m[3][3] - m[2][3] * m[3][1];
	float jokn = m[2][1] * m[3][2] - m[2][2] * m[3][1];
	float iplm = m[2][0] * m[3][3] - m[2][3] * m[3][0];
	float iokm = m[2][0] * m[3][2] - m[2][2] * m[3][0];
	float injm = m[2][0] * m[3][1] - m[2][1] * m[3][0];

	float gpho = m[1][2] * m[3][3] - m[1][3] * m[3][2];
	float fphn = m[1][1] * m[3][3] - m[1][3] * m[3][1];
	float fogn = m[1][1] * m[3][2] - m[1][2] * m[3][1];
	float ephm = m[1][0] * m[3][3] - m[1][3] * m[3][0];
	float eogm = m[1][0] * m[3][2] - m[1][2] * m[3][0];
	float enfm = m[1][0] * m[3][1] - m[1][1] * m[3][0];

	float glhk = m[1][2] * m[2][3] - m[1][3] * m[2][2];
	float flhj = m[1][1] * m[2][3] - m[1][3] * m[2][1];
	float fkgj = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	float elhi = m[1][0] * m[2][3] - m[1][3] * m[2][0];
	float ekgi = m[1][0] * m[2][2] - m[1][2] * m[2][0];
	float ejfi = m[1][0] * m[2][1] - m[1][1] * m[2][0];

	float p[4][4] =
	{
		{
			(m[1][1] * kplo - m[1][2] * jpln + m[1][3] * jokn),
			-(m[1][0] * kplo - m[1][2] * iplm + m[1][3] * iokm),
			(m[1][0] * jpln - m[1][1] * iplm + m[1][3] * injm),
			-(m[1][0] * jokn - m[1][1] * iokm + m[1][2] * injm)
		},
		{
			-(m[0][1] * kplo - m[0][2] * jpln + m[0][3] * jokn),
			(m[0][0] * kplo - m[0][2] * iplm + m[0][3] * iokm),
			-(m[0][0] * jpln - m[0][1] * iplm + m[0][3] * injm),
			(m[0][0] * jokn - m[0][1] * iokm + m[0][2] * injm)
		},
		{
			(m[0][1] * gpho - m[0][2] * fphn + m[0][3] * fogn),
			-(m[0][0] * gpho - m[0][2] * ephm + m[0][3] * eogm),
			(m[0][0] * fphn - m[0][1] * ephm + m[0][3] * enfm),
			-(m[0][0] * fogn - m[0][1] * eogm + m[0][2] * enfm)
		},
		{
			-(m[0][1] * glhk - m[0][2] * flhj + m[0][3] * fkgj),
			(m[0][0] * glhk - m[0][2] * elhi + m[0][3] * ekgi),
			-(m[0][0] * flhj - m[0][1] * elhi + m[0][3] * ejfi),
			(m[0][0] * fkgj - m[0][1] * ekgi + m[0][2] * ejfi)
		}
	};

	return Matrix( p );
}

float Matrix::GetCofactorAt( int i, int j ) const
{
	// Why not just use "sign = ( i % 2 == 0 ) ? 1 : -1"?
	// Also, do I even use this anywhere?
	int sign = ( ( 1 - ( ( i + j ) & 1 ) ) << 1 ) - 1;	// sign = 1 if i+j is even or -1 if i+j is odd
	int i1 = (i + 1) & 3;								// & 3 = % 4
	int i2 = (i + 2) & 3;
	int i3 = (i + 3) & 3;
	int j1 = (j + 1) & 3;
	int j2 = (j + 2) & 3;
	int j3 = (j + 3) & 3;

	float term1 = m[i1][j1] * (m[i2][j2] * m[i3][j3] - m[i2][j3] * m[i3][j2]);
	float term2 = -(m[i1][j2] * (m[i2][j1] * m[i3][j3] - m[i2][j3] * m[i3][j1]));
	float term3 = m[i1][j3] * (m[i2][j1] * m[i3][j2] - m[i2][j2] * m[i3][j1]);

	return ( sign * ( term1 + term2 + term3 ) );
}

void Matrix::ZeroTranslationElements()
{
	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = 0.f;
}

Vector Matrix::GetTranslationElements() const
{
	return Vector( m[3][0], m[3][1], m[3][2] );
}

void Matrix::SetTranslationElements( const Vector& v )
{
	m[3][0] = v.x;
	m[3][1] = v.y;
	m[3][2] = v.z;
}

void Matrix::Identity()
{
	m[0][0] = 1.f;
	m[0][1] = 0.f;
	m[0][2] = 0.f;
	m[0][3] = 0.f;
	m[1][0] = 0.f;
	m[1][1] = 1.f;
	m[1][2] = 0.f;
	m[1][3] = 0.f;
	m[2][0] = 0.f;
	m[2][1] = 0.f;
	m[2][2] = 1.f;
	m[2][3] = 0.f;
	m[3][0] = 0.f;
	m[3][1] = 0.f;
	m[3][2] = 0.f;
	m[3][3] = 1.f;
}

Quat Matrix::ToQuaternion() const
{
	// Based on the following, but flipped for row-major:
	// http://www.j3d.org/matrix_faq/matrfaq_latest.html#Q54
	float Trace = 1.0f + m[0][0] + m[1][1] + m[2][2];
	float w, x, y, z;
	if( Trace > 0.0f )
	{
		float S = SqRt( Trace ) * 2.0f;
		w = 0.25f * S;
		x = ( m[1][2] - m[2][1] ) / S;
		y = ( m[2][0] - m[0][2] ) / S;
		z = ( m[0][1] - m[1][0] ) / S;
	}
	else
	{
		if( m[0][0] > m[1][1] && m[0][0] > m[2][2] )
		{
			float S = SqRt( 1.0f + m[0][0] - m[1][1] - m[2][2] ) * 2.0f;
			x = 0.25f * S;
			w = (m[1][2] - m[2][1] ) / S;
			y = (m[0][1] + m[1][0] ) / S;
			z = (m[2][0] + m[0][2] ) / S;
		}
		else if( m[1][1] > m[2][2] )
		{
			float S = SqRt( 1.0f + m[1][1] - m[0][0] - m[2][2] ) * 2.0f;
			y = 0.25f * S;
			w = (m[2][0] - m[0][2] ) / S;
			x = (m[0][1] + m[1][0] ) / S;
			z = (m[1][2] + m[2][1] ) / S;
		}
		else
		{
			float S = SqRt( 1.0f + m[2][2] - m[0][0] - m[1][1] ) * 2.0f;
			z = 0.25f * S;
			w = (m[0][1] - m[1][0] ) / S;
			x = (m[2][0] + m[0][2] ) / S;
			y = (m[1][2] + m[2][1] ) / S;
		}
	}
	return Quat( w, x, y, z );
}

/*static*/ Matrix Matrix::CreateCoordinate( const Vector& X, const Vector& Y, const Vector& Z )
{
	return Matrix(
		X.x, X.y, X.z, 0.0f,
		Y.x, Y.y, Y.z, 0.0f,
		Z.x, Z.y, Z.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f );
}

Matrix Matrix::CreateScale( const Vector& s )
{
	return Matrix(
		s.x, 0, 0, 0,
		0, s.y, 0, 0,
		0, 0, s.z, 0,
		0, 0, 0, 1.f );
}

Matrix Matrix::CreateSkewXY( float x, float y )
{
	return Matrix(
		1.f, 0, x, 0,
		0, 1.f, y, 0,
		0, 0, 1.f, 0,
		0, 0, 0, 1.f );
}

Matrix Matrix::CreateSkewXZ( float x, float z )
{
	return Matrix(
		1.f, x, 0, 0,
		0, 1.f, 0, 0,
		0, z, 1.f, 0,
		0, 0, 0, 1.f );
}

Matrix Matrix::CreateSkewYZ( float y, float z)
{
	return Matrix(
		1.f, 0, 0, 0,
		y, 1.f, 0, 0,
		z, 0, 1.f, 0,
		0, 0, 0, 1.f );
}

// Perhaps use a table of trig functions to improve the speed here...
Matrix Matrix::CreateRotationAboutX( float theta )
{
	float c = Cos( theta );
	float s = Sin( theta );
	return Matrix(
		1.f, 0, 0, 0,
		0, c, s, 0,
		0, -s, c, 0,
		0, 0, 0, 1.f );
}

Matrix Matrix::CreateRotationAboutY( float theta )
{
	float c = Cos( theta );
	float s = Sin( theta );
	return Matrix(
		c, 0, -s, 0,
		0, 1.f, 0, 0,
		s, 0, c, 0,
		0, 0, 0, 1.f );
}

Matrix Matrix::CreateRotationAboutZ( float theta )
{
	float c = Cos( theta );
	float s = Sin( theta );
	return Matrix(
		c, s, 0, 0,
		-s, c, 0, 0,
		0, 0, 1.f, 0,
		0, 0, 0, 1.f );
}

Matrix Matrix::CreateRotation( const Vector& axis, float theta )
{
	float c = Cos( theta );
	float s = Sin( theta );
	float oc = ( 1.f - c );
	Vector t( axis.GetNormalized() );
	float xy = t.x * t.y;
	float xz = t.x * t.z;
	float yz = t.y * t.z;

	return Matrix(
		t.x * t.x * oc + c,		xy * oc + t.z * s,		xz * oc - t.y * s,		0.0f,
		xy * oc - t.z * s,		t.y * t.y * oc + c,		yz * oc + t.x * s,		0.0f,
		xz * oc + t.y * s,		yz * oc - t.x * s,		t.z * t.z * oc + c,		0.0f,
		0.0f,					0.0f,					0.0f,					1.0f );
}

Matrix Matrix::CreateTranslation( const Vector& t )
{
	return Matrix(
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		t.x, t.y, t.z, 1.f );
}

// The parameter is the normal to the plane to project onto
Matrix Matrix::CreateProjectionOnto( const Vector& n )
{
	Vector t( n.GetNormalized() );
	float xy = -t.x * t.y;
	float xz = -t.x * t.z;
	float yz = -t.y * t.z;

	return Matrix(
		1.f - t.x * t.x, xy, xz, 0.f,
		xy, 1.f - t.y * t.y, yz, 0.f,
		xz, yz, 1.f - t.z * t.z, 0.f,
		0.f, 0.f, 0.f, 1.f );
}

// The parameter is the normal to the plane to reflect about
Matrix Matrix::CreateReflection( const Vector& n)
{
	Vector t( n.GetNormalized() );
	float xy = -2.0f * t.x * t.y;
	float xz = -2.0f * t.x * t.z;
	float yz = -2.0f * t.y * t.z;

	return Matrix(
		1.f - 2.f * t.x * t.x, xy, xz, 0.f,
		xy, 1.f - 2.f * t.y * t.y, yz, 0.f,
		xz, yz, 1.f - 2.f * t.z * t.z, 0.f,
		0.f, 0.f, 0.f, 1.f );
}

Matrix Matrix::CreateViewMatrixDirection( const Vector& eye, const Vector& dir )
{
	Vector up( 0.f, 0.f, 1.f );
	Vector fd = dir.GetNormalized();
	Vector rt = fd.Cross( up ).GetNormalized();
	up = rt.Cross( fd );

	return Matrix(
		rt.x, up.x, -fd.x, 0.f,
		rt.y, up.y, -fd.y, 0.f,
		rt.z, up.z, -fd.z, 0.f,
		-(rt.Dot(eye)), -(up.Dot(eye)), (fd.Dot(eye)), 1.f );
}

Matrix Matrix::CreateViewMatrixLookAt( const Vector& eye, const Vector& at )
{
	return CreateViewMatrixDirection( eye, at - eye );
}

Matrix Matrix::CreateViewMatrixCoords( const Vector& eye, const Vector& rt, const Vector& fd, const Vector& up )
{
	ASSERT( Equal( rt.LengthSquared(), 1.0f ) );
	ASSERT( Equal( fd.LengthSquared(), 1.0f ) );
	ASSERT( Equal( up.LengthSquared(), 1.0f ) );
	return Matrix(
		rt.x, up.x, -fd.x, 0.0f,
		rt.y, up.y, -fd.y, 0.0f,
		rt.z, up.z, -fd.z, 0.0f,
		-(rt.Dot(eye)), -(up.Dot(eye)), (fd.Dot(eye)), 1.0f );
}

Matrix Matrix::CreateProjectionMatrix( float fov, float nearC, float farC, float aRatio )
{
	float invR = 1.f / ( farC - nearC );

	return CreateProjectionMatrix( fov, nearC, farC, invR, aRatio );
}

Matrix Matrix::CreateProjectionMatrix( float fov, float nearC, float farC, float invR, float aRatio )
{
	float rFOV = DEGREES_TO_RADIANS( fov );
	float yScale = 1.f / ( Tan( rFOV * 0.5f ) );
	float xScale = yScale / aRatio;
	float z = farC * -invR;
	float z2 = nearC * z;

	return Matrix(
		xScale,	0.f,	0.f,	0.f,
		0.f,	yScale,	0.f,	0.f,
		0.f,	0.f,	z,		-1.f,
		0.f,	0.f,	z2,		0.f);
}

Matrix operator*( float f, const Matrix& n )
{
	return n * f;
}

Matrix operator/( float f, const Matrix& n )
{
	return n / f;
}

// Assumes this is just a rotation matrix
Angles Matrix::ToAngles() const
{
	float sy = -m[0][2];
	float cy = SqRt( 1 - ( sy * sy ) );
	float sx, cx, sz, cz;

	if( cy < EPSILON )
	{
		sz = 0.f;
		cz = 1.f;
		sx = -m[2][1];
		cx = m[1][1];
	}
	else
	{
		float rcy = 1.f / cy;
		sx = m[1][2] * rcy;
		cx = m[2][2] * rcy;
		sz = m[0][1] * rcy;
		cz = m[0][0] * rcy;
	}

	float Pitch = atan2( sx, cx );
	float Roll = atan2( sy, cy );
	float Yaw = atan2( sz, cz );

	return Angles( Pitch, Roll, Yaw );
}

Matrix Matrix::CreateOrthoProjectionMatrix( float Left, float Top, float Right, float Bottom, float NearZ, float InvRange )
{
	return Matrix(
		2.0f/(Right-Left),			0.0f,						0.0f,				0.0f,
		0.0f,						2.0f/(Top-Bottom),			0.0f,				0.0f,
		0.0f,						0.0f,						-InvRange,			0.0f,
		(Left+Right)/(Left-Right),	(Top+Bottom)/(Bottom-Top),	NearZ*-InvRange,	1.0f);
}