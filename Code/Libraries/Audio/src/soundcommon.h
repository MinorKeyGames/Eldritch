#ifndef SOUNDCOMMON_H
#define SOUNDCOMMON_H

#include "isound.h"
#include "mathcore.h"

class ISoundInstance;
class IAudioSystem;
struct SSoundInit;

class SoundCommon : public ISound
{
public:
	SoundCommon();
	virtual ~SoundCommon();

	virtual IAudioSystem*			GetAudioSystem() const						{ return m_System; }
	virtual bool					GetIsStream() const							{ return m_IsStream; }
	virtual bool					GetIs3D() const								{ return m_Is3D; }
	virtual float					GetBaseVolume() const						{ return m_BaseVolume; }
	virtual float					GetFalloffDistance() const					{ return m_FalloffRadius; }
	virtual float					GetMinimumAttenuation() const				{ return m_MinimumAttenuation; }
	virtual float					GetBiasedPan( const float Distance ) const	{ return Saturate( ( Distance - m_PanBiasNear ) * m_InversePanBiasRange ); }
	virtual float					GetPanPower() const							{ return m_PanPower; }
	virtual float					GetRearAttenuation() const					{ return m_RearAttenuation; }
	virtual const HashedString&		GetCategory() const							{ return m_Category; }
	virtual ESoundPriority			GetPriority() const							{ return m_Priority; }
	virtual bool					ShouldCalcOcclusion() const					{ return m_ShouldCalcOcclusion; }

	virtual float					GetCategoryVolume() const;
	virtual const Sound3DListener*	Get3DListener() const;
	virtual float					GetMasterVolume() const;

	void							Initialize( const SSoundInit& SoundInit );

	void							SetAudioSystem( IAudioSystem* const pSystem ) { m_System = pSystem; }
	void							SetIsStream( const bool Stream ) { m_IsStream = Stream; }
	void							SetCategory( const HashedString& Category ) { m_Category = Category; }
	void							SetPriority( const ESoundPriority Priority ) { m_Priority = Priority; }
	void							SetBaseVolume( const float Volume ) { m_BaseVolume = Volume; }
	void							SetFalloffRadius( const float FalloffRadius ) { m_FalloffRadius = FalloffRadius; }
	void							SetPanBias( const float Near, const float Far ) { m_PanBiasNear = Near; m_InversePanBiasRange = 1.0f / ( Far - Near ); }
	void							SetPanPower( const float Power ) { m_PanPower = Power; }
	void							SetMinimumAttenuation( const float Attenuation ) { m_MinimumAttenuation = Attenuation; }
	void							SetRearAttenuation( const float Attenuation ) { m_RearAttenuation = Attenuation; }
	void							SetShouldCalcOcclusion( const bool ShouldCalcOcclusion ) { m_ShouldCalcOcclusion = ShouldCalcOcclusion; }

	void							SetLooping( const bool Looping ) { m_Looping = Looping; }
	bool							GetLooping() const { return m_Looping; }

	void							SetIs3D( const bool Is3D ) { m_Is3D = Is3D; }

private:
	IAudioSystem*				m_System;

	bool						m_IsStream;
	bool						m_Looping;
	bool						m_Is3D;
	bool						m_ShouldCalcOcclusion;
	float						m_BaseVolume;
	float						m_FalloffRadius;		// At Radius, the volume is halved.
	float						m_PanBiasNear;			// Within Near, pan is centered. At Far, pan is actual pan.
	float						m_InversePanBiasRange;
	float						m_PanPower;
	float						m_MinimumAttenuation;
	float						m_RearAttenuation;		// At 1.0, rear attenuation is full. At 0.0, there is no rear attenuation.
	HashedString				m_Category;
	ESoundPriority				m_Priority;
};

#endif // SOUNDCOMMON_H