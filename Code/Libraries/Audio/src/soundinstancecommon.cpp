#include "core.h"
#include "soundinstancecommon.h"
#include "isound.h"
#include "sound3dlistener.h"
#include "mathcore.h"

SoundInstanceCommon::SoundInstanceCommon()
:	m_Sound( NULL )
,	m_Attenuation( 1.0f )
,	m_BaseVolume( 1.0f )
,	m_Location()
{
}

SoundInstanceCommon::~SoundInstanceCommon()
{
}

/*virtual*/ void SoundInstanceCommon::Tick()
{
	PROFILE_FUNCTION;

	if( GetSound()->GetIs3D() )
	{
		Tick3D();
	}

	SetVolume( m_Attenuation * GetBaseVolume() * GetSound()->GetBaseVolume() * GetCategoryVolume() * GetMasterVolume() );
}

/*virtual*/ void SoundInstanceCommon::Tick3D()
{
	DEBUGASSERT( GetSound()->GetIs3D() );

	const Sound3DListener* const pListener = Get3DListener();
	DEVASSERT( pListener );

	Vector	DirectionToSound;
	float	DistanceToSound;
	float	OneOverDistanceToSound;

	const Vector	SoundLocation		= GetLocation();
	const Vector	OffsetToSound		= SoundLocation - pListener->GetLocation();
	OffsetToSound.GetNormalized( DirectionToSound, DistanceToSound, OneOverDistanceToSound );

	const float		FalloffRadius		= m_Sound->GetFalloffDistance();
	const float		MinimumAttenuation	= m_Sound->GetMinimumAttenuation();
	DEVASSERT( FalloffRadius > 0.0f );

	// Set pan based on distance and direction.
	const float		PanBias				= m_Sound->GetBiasedPan( DistanceToSound );
	const float		PanCosTheta			= pListener->GetRight().Dot( DirectionToSound );

	// This is a little bit of fakery; further attenuate sounds behind the listener.
	const float	Surround				= pListener->GetForward().Dot( DirectionToSound );
	float		RearAttenuation			= Saturate( ( -Surround * PanBias ) );
	RearAttenuation = 1.0f - ( RearAttenuation * m_Sound->GetRearAttenuation() );

	// Set attenuation based on distance and rear attenuation
	m_Attenuation						= FalloffRadius / ( FalloffRadius + DistanceToSound );
	m_Attenuation *= RearAttenuation;

	if( m_Attenuation >= MinimumAttenuation )
	{
		pListener->ModifyAttenuation( this, m_Attenuation );
	}

	if( m_Attenuation < MinimumAttenuation )
	{
		m_Attenuation = 0.0f;
	}
	else
	{
		// Don't bother setting the pan unless we'll hear it!
		const float PanPow	= SignedPow( PanCosTheta, m_Sound->GetPanPower() );
		const float Pan		= PanPow * PanBias;
		SetPan( Pan );
	}
}

/*virtual*/ const HashedString& SoundInstanceCommon::GetCategory() const
{
	DEVASSERT( m_Sound );
	return m_Sound->GetCategory();
}

/*virtual*/ float SoundInstanceCommon::GetCategoryVolume() const
{
	DEVASSERT( m_Sound );
	return m_Sound->GetCategoryVolume();
}

/*virtual*/ const Sound3DListener* SoundInstanceCommon::Get3DListener() const
{
	DEVASSERT( m_Sound );
	return m_Sound->Get3DListener();
}

/*virtual*/ float SoundInstanceCommon::GetMasterVolume() const
{
	DEVASSERT( m_Sound );
	return m_Sound->GetMasterVolume();
}

/*virtual*/ bool SoundInstanceCommon::ShouldCalcOcclusion() const
{
	DEVASSERT( m_Sound );
	return m_Sound->ShouldCalcOcclusion();
}