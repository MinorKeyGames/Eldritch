#ifndef ELDRITCHPARTICLES_H
#define ELDRITCHPARTICLES_H

// Bunch of stuff in here copied from ParticleSystem; this is different
// because it's a resource which will be used by a component, like a Mesh is.

#include "vector.h"
#include "vector4.h"
#include "vector2.h"
#include "angles.h"
#include "list.h"
#include "array.h"
#include "matrix.h"
#include "3d.h"
#include "map.h"

class EldritchMesh;
class ITexture;
class IVertexBuffer;

class EldritchParticles
{
public:
	EldritchParticles();
	~EldritchParticles();

	void			InitializeFromDefinition( const SimpleString& DefinitionName );

	void			Tick( const float DeltaTime );
	void			Render();

	Vector			GetLocation() const { return m_SystemLocation; }
	void			SetLocation( const Vector& Location ) { m_SystemLocation = Location; }
	void			SetOrientation( const Angles& Orientation ) { m_SystemOrientation = Orientation; }

	bool			IsFinished() const;

	static void		FlushConfigCache();

private:
	struct SParticle
	{
		SParticle();

		bool operator<( const SParticle& P ) { return m_ViewDistanceSq > P.m_ViewDistanceSq; }	// Further particles belong first in list

		float	m_Lifetime;				// If <= 0, particle is permanent
		float	m_Size;
		float	m_SizeVelocity;
		Vector4	m_Color;
		Vector	m_InitialOffset;		// Offset from base emitter location
		Vector	m_Location;
		Vector	m_Velocity;
		float	m_ViewDistanceSq;		// For sorting
	};

	struct SParticleMeshParams 
	{
		SParticleMeshParams()
			:	m_ViewLocation()
			,	m_ViewAxis()
			,	m_AngleMatrix()
			,	m_OriginMatrix()
		{
		}

		Vector	m_ViewLocation;
		Vector	m_ViewAxis;
		Matrix	m_AngleMatrix;
		Matrix	m_OriginMatrix;
	};

	void			CreateMesh();
	void			InitVertexBuffers( IVertexBuffer* pVertexBuffer );
	void			UpdateMesh();
	void			AddParticleToMesh( const uint Index, const SParticle& Particle, const SParticleMeshParams& MeshParams );
	void			TickParticles( const float DeltaTime );
	bool			TickParticle( SParticle& Particle, float DeltaTime );	// Returns false if this particle has expired
	float			GetViewDistanceSq( const Vector& ParticleLocation );
	uint			GetNumParticles() const;
	void			SpawnParticles( uint NumParticlesToSpawn );
	Vector			GetParticleSpawnLocationOffset() const;

	static void		DeviceResetCallback( void* pVoid, IVertexBuffer* pBuffer );

	void			GetCachedConfig( const SimpleString& DefinitionName );

	float			GetTime() const;
	bool			SystemIsFinite() const;
	bool			IsExpired() const;
	bool			IsUnsorted() const;

	// Configurable parameters
	struct SParticleSystemParams
	{
		SParticleSystemParams()
		:	m_SystemLifetime( 0.0f )
		,	m_SpawnRate( 0.0f )
		,	m_InvSpawnRate( 0.0f )
		,	m_MaxParticles( 0 )
		,	m_TextureMapName( NULL )
		,	m_ImmediateSpawnMax( false )
		,	m_AdditiveDraw( false )
		,	m_AlphaTestDraw( false )
		,	m_AlwaysDraw( false )
		,	m_ForegroundDraw( false )
		,	m_Collision( false )
		,	m_SpawnOffsetOS()
		,	m_SpawnOffsetWS()
		,	m_SpawnExtents()
		,	m_InitialVelocityOSMin()
		,	m_InitialVelocityOSMax()
		,	m_Elasticity( 0.0f )
		,	m_LifetimeMin( 0.0f )
		,	m_LifetimeMax( 0.0f )
		,	m_AccelerationWS()
		,	m_ViewBoundRadius( 0.0f )
		,	m_InitialSizeMin( 0.0f )
		,	m_InitialSizeMax( 0.0f )
		,	m_SizeVelocityMin( 0.0f )
		,	m_SizeVelocityMax( 0.0f )
		,	m_ColorMin()
		,	m_ColorMax()
		,	m_LinkedRGB( false )
		,	m_InitialTrace( false )
		,	m_InitialTraceOffsetOS()
		{
		}

		float			m_SystemLifetime;
		float			m_SpawnRate;
		float			m_InvSpawnRate;
		uint			m_MaxParticles;
		const char*		m_TextureMapName;
		bool			m_ImmediateSpawnMax;
		bool			m_AdditiveDraw;
		bool			m_AlphaTestDraw;
		bool			m_AlwaysDraw;
		bool			m_ForegroundDraw;
		bool			m_Collision;
		Vector			m_SpawnOffsetOS;
		Vector			m_SpawnOffsetWS;
		Vector			m_SpawnExtents;
		Vector			m_InitialVelocityOSMin;
		Vector			m_InitialVelocityOSMax;
		float			m_Elasticity;
		float			m_LifetimeMin;
		float			m_LifetimeMax;
		Vector			m_AccelerationWS;
		float			m_ViewBoundRadius;
		float			m_InitialSizeMin;
		float			m_InitialSizeMax;
		float			m_SizeVelocityMin;
		float			m_SizeVelocityMax;
		Vector4			m_ColorMin;
		Vector4			m_ColorMax;
		bool			m_LinkedRGB;
		bool			m_InitialTrace;
		Vector			m_InitialTraceOffsetOS;
	};

	SParticleSystemParams	m_Params;

	Vector					m_SystemLocation;		// Emitter base location
	Angles					m_SystemOrientation;

	float					m_SpawnAccumulator;
	float					m_ExpireTime;
	float					m_FinishTime;
	bool					m_RenderedLastFrame;
	bool					m_HasImmediateSpawnedMax;

	EldritchMesh*			m_DynamicMesh;
	ITexture*				m_TextureMap;

	// Use a list for non-additive particles that need to be sorted and an
	// array for additive and alpha-test particles because it should be faster.
	List<SParticle>			m_ParticlesList;
	Array<SParticle>		m_ParticlesArray;

	// Persistently allocated local vertex buffers for pushing each frame
	Array<Vector>			m_VB_Positions;
	Array<Vector4>			m_VB_FloatColors;
	Array<Vector2>			m_VB_UVs;
	Array<Vector>			m_VB_Normals;
	Array<index_t>			m_VB_Indices;

	typedef Map<HashedString, EldritchParticles::SParticleSystemParams> TParamCache;
	static TParamCache		sm_ParamCache;
};

#endif // ELDRITCHPARTICLES_H