#include "core.h"
#include "soundcommon.h"
#include "idatastream.h"
#include "iaudiosystem.h"

SoundCommon::SoundCommon()
:	m_System( NULL )
,	m_IsStream( false )
,	m_Looping( false )
,	m_Is3D( false )
,	m_ShouldCalcOcclusion( false )
,	m_BaseVolume( 0.0f )
,	m_FalloffRadius( 0.0f )
,	m_PanBiasNear( 0.0f )
,	m_InversePanBiasRange( 0.0f )
,	m_PanPower( 0.0f )
,	m_MinimumAttenuation( 0.0f )
,	m_RearAttenuation( 0.0f )
,	m_Category()
,	m_Priority( ESP_Default )
{
}

SoundCommon::~SoundCommon()
{
}

void SoundCommon::Initialize( const SSoundInit& SoundInit )
{
	SetPriority(			SoundInit.m_Priority );
	SetIsStream(			SoundInit.m_IsStream );
	SetCategory(			SoundInit.m_Category );
	SetLooping(				SoundInit.m_IsLooping );
	SetIs3D(				SoundInit.m_Is3D );
	SetShouldCalcOcclusion(	SoundInit.m_ShouldCalcOcclusion );
	SetBaseVolume(			SoundInit.m_Volume );
	SetFalloffRadius(		SoundInit.m_FalloffDistance );
	SetPanBias(				SoundInit.m_PanBiasNear, SoundInit.m_PanBiasFar );
	SetPanPower(			SoundInit.m_PanPower );
	SetMinimumAttenuation(	SoundInit.m_MinimumAttenuation );
	SetRearAttenuation(		SoundInit.m_RearAttenuation );
}

/*virtual*/ float SoundCommon::GetCategoryVolume() const
{
	DEVASSERT( m_System );
	return m_System->GetCategoryVolume( m_Category );
}

/*virtual*/ float SoundCommon::GetMasterVolume() const
{
	DEVASSERT( m_System );
	return m_System->GetMasterVolume();
}

/*virtual*/ const Sound3DListener* SoundCommon::Get3DListener() const
{
	DEVASSERT( m_System );
	return m_System->Get3DListener();
}