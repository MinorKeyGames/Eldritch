#ifndef WBCOMPELDPARTICLES_H
#define WBCOMPELDPARTICLES_H

#include "wbeldritchcomponent.h"
#include "array.h"
#include "simplestring.h"
#include "vector.h"
#include "angles.h"

class EldritchParticles;

class WBCompEldParticles : public WBEldritchComponent
{
public:
	WBCompEldParticles();
	virtual ~WBCompEldParticles();

	DEFINE_WBCOMP( EldParticles, WBEldritchComponent );

	virtual void	Tick( float DeltaTime );
	virtual int		GetTickOrder() { return ETO_TickDefault; }	// Should tick after transform

	virtual bool	IsRenderable() { return true; }
	virtual void	Render();

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

protected:
	virtual void	InitializeFromDefinition( const SimpleString& DefinitionName );

private:
	void			AddParticleSystem( const SimpleString& DefinitionName, const bool Serialize, const bool Attached, const Vector* const pLocationOverride );
	void			AddParticleSystem( const SimpleString& DefinitionName, const bool Serialize, const bool Attached, const Vector& Location, const Angles& Orientation );
	void			StopParticleSystem( const HashedString& DefinitionNameHash );

	struct SParticleSystem
	{
		SParticleSystem()
		:	m_System( NULL )
		,	m_Attached( false )
		,	m_Serialize( false )
		,	m_DefinitionName()
		,	m_Location()
		,	m_Orientation()
		,	m_DefinitionNameHash()
		{
		}

		EldritchParticles*	m_System;
		bool				m_Attached;

		// Initialization parameters, so we can serialize active systems
		bool				m_Serialize;
		SimpleString		m_DefinitionName;
		Vector				m_Location;
		Angles				m_Orientation;

		// For quick removal
		HashedString		m_DefinitionNameHash;
	};

	Array<SParticleSystem>	m_ParticleSystems;
	bool					m_Hidden;

	float					m_CullDistanceSq;	// Config: If non-zero, mesh is only drawn within this distance from camera
};

#endif // WBCOMPELDPARTICLES_H