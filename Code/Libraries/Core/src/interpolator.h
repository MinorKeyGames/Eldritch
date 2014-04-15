#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "mathcore.h"

// Linear and Hermite curve interpolations defined by a starting value,
// ending value, time range, and interpolation type.

template<class C> class Interpolator
{
public:
	enum EInterpolationType
	{
		EIT_None,
		EIT_Linear,
		EIT_EaseIn,
		EIT_EaseOut,
		EIT_EaseInOut,
	};

	Interpolator(
		EInterpolationType	InterpolationType = EIT_None,
		const C&			StartValue = C(),
		const C&			EndValue = C(),
		float				Duration = 0.0f )
	{
		Reset( InterpolationType, StartValue, EndValue, Duration );
	}

	~Interpolator()
	{
	}

	void		Reset(
		EInterpolationType	InterpolationType,
		const C&			StartValue,
		const C&			EndValue,
		float				Duration )
	{
		m_InterpolationType	= InterpolationType;
		m_CurrentValue		= ( Duration <= 0.0f ) ? EndValue : StartValue;	// So the reset takes even before the first tick
		m_StartValue		= StartValue;
		m_EndValue			= EndValue;
		m_Duration			= Duration;
		m_ElapsedTime		= 0.0f;
	}

	float		GetT() const
	{
		return ( m_Duration > 0.0f ) ? Saturate( m_ElapsedTime / m_Duration ) : 1.0f;
	}

	void		Tick( float DeltaTime )
	{
		if( m_InterpolationType == EIT_None )
		{
			return;
		}

		m_ElapsedTime += DeltaTime;

		float t = GetT();

		// Simple cases
		if( t == 0.0f )
		{
			m_CurrentValue = m_StartValue;
			return;
		}
		else if( t == 1.0f )
		{
			m_CurrentValue = m_EndValue;
			return;
		}

		// Modify the t-value so we can just do a LERP on the actual values
		if( m_InterpolationType == EIT_Linear )
		{
			// Do nothing, just use t straight
		}
		else if( m_InterpolationType == EIT_EaseIn )
		{
			// Fixed Hermite curve between 0 and 1 with tangents 0 and 1
			float h01 = t * t * ( 3.0f - 2.0f * t );
			float h11 = t * t * ( t - 1.0f );

			t = h01 + h11;
		}
		else if( m_InterpolationType == EIT_EaseOut )
		{
			// Fixed Hermite curve between 0 and 1 with tangents 1 and 0
			float h10 = t * ( 1.0f - t ) * ( 1.0f - t );
			float h01 = t * t * ( 3.0f - 2.0f * t );

			t = h10 + h01;
		}
		else if( m_InterpolationType == EIT_EaseInOut )
		{
			// Fixed Hermite curve between 0 and 1 with tangents 0 and 0
			t = t * t * ( 3.0f - 2.0f * t );
		}

		// LERP the values using the modified t-value
		m_CurrentValue = m_StartValue * ( 1.0f - t ) + m_EndValue * t;
	}

	const C&	GetValue() const
	{
		return m_CurrentValue;
	}

	bool		IsFinished() const
	{
		return m_ElapsedTime >= m_Duration;
	}

private:
	EInterpolationType	m_InterpolationType;
	C					m_StartValue;
	C					m_EndValue;
	float				m_Duration;

	C					m_CurrentValue;
	float				m_ElapsedTime;
};

#endif // INTERPOLATOR_H