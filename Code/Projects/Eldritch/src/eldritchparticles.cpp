#include "core.h"
#include "eldritchparticles.h"
#include "eldritchframework.h"
#include "eldritchmesh.h"
#include "mathfunc.h"
#include "collisioninfo.h"
#include "eldritchworld.h"
#include "segment.h"
#include "ivertexdeclaration.h"
#include "configmanager.h"
#include "view.h"
#include "irenderer.h"
#include "texturemanager.h"
#include "mathcore.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "shadermanager.h"
#include "wbworld.h"
#include "eldritchgame.h"

/*static*/ EldritchParticles::TParamCache EldritchParticles::sm_ParamCache;

EldritchParticles::EldritchParticles()
:	m_Params()
,	m_SystemLocation()
,	m_SystemOrientation()
,	m_SpawnAccumulator( 0.0f )
,	m_ExpireTime( 0.0f )
,	m_FinishTime( 0.0f )
,	m_RenderedLastFrame( false )
,	m_HasImmediateSpawnedMax( false )
,	m_DynamicMesh( NULL )
,	m_TextureMap( NULL )
,	m_ParticlesList()
,	m_ParticlesArray()
,	m_VB_Positions()
,	m_VB_FloatColors()
,	m_VB_UVs()
,	m_VB_Normals()
,	m_VB_Indices()
{
}

EldritchParticles::~EldritchParticles()
{
	IRenderer* const pRenderer = EldritchFramework::GetInstance()->GetRenderer();

	if( m_DynamicMesh && pRenderer )
	{
		pRenderer->RemoveDynamicVertexBuffer( m_DynamicMesh->m_VertexBuffer );
	}

	SafeDelete( m_DynamicMesh );
}

EldritchParticles::SParticle::SParticle()
:	m_Lifetime( 0.0f )
,	m_Size( 0.0f )
,	m_SizeVelocity( 0.0f )
,	m_Color()
,	m_InitialOffset()
,	m_Location()
,	m_Velocity()
,	m_ViewDistanceSq( 0.0f )
{
}

void EldritchParticles::GetCachedConfig( const SimpleString& DefinitionName )
{
	const HashedString HashedDefinitionName = DefinitionName;
	Map<HashedString, SParticleSystemParams>::Iterator CacheIter = sm_ParamCache.Search( HashedDefinitionName );

	if( CacheIter.IsValid() )
	{
		m_Params = CacheIter.GetValue();
	}
	else
	{
		MAKEHASH( DefinitionName );

		STATICHASH( SystemLifetime );
		m_Params.m_SystemLifetime = ConfigManager::GetInheritedFloat( sSystemLifetime, 0.0f, sDefinitionName );
		
		STATICHASH( SpawnRate );
		m_Params.m_SpawnRate = ConfigManager::GetInheritedFloat( sSpawnRate, 0.0f, sDefinitionName );
		m_Params.m_InvSpawnRate = ( m_Params.m_SpawnRate > 0.0f ) ? ( 1.0f / m_Params.m_SpawnRate ) : 0.0f;
		
		STATICHASH( MaxParticles );
		m_Params.m_MaxParticles = (uint)ConfigManager::GetInheritedInt( sMaxParticles, 1, sDefinitionName );
		
		STATICHASH( TextureMap );
		m_Params.m_TextureMapName = ConfigManager::GetInheritedString( sTextureMap, DEFAULT_TEXTURE, sDefinitionName );
		
		STATICHASH( ImmediateSpawnMax );
		m_Params.m_ImmediateSpawnMax = ConfigManager::GetInheritedBool( sImmediateSpawnMax, false, sDefinitionName );

		STATICHASH( AdditiveDraw );
		m_Params.m_AdditiveDraw = ConfigManager::GetInheritedBool( sAdditiveDraw, false, sDefinitionName );
		
		STATICHASH( AlphaTestDraw );
		m_Params.m_AlphaTestDraw = ConfigManager::GetInheritedBool( sAlphaTestDraw, false, sDefinitionName );
		
		STATICHASH( AlwaysDraw );
		m_Params.m_AlwaysDraw = ConfigManager::GetInheritedBool( sAlwaysDraw, false, sDefinitionName );

		STATICHASH( ForegroundDraw );
		m_Params.m_ForegroundDraw = ConfigManager::GetInheritedBool( sForegroundDraw, false, sDefinitionName );
		
		STATICHASH( Collision );
		m_Params.m_Collision = ConfigManager::GetInheritedBool( sCollision, false, sDefinitionName );
		
		STATICHASH( SpawnOffsetOSX );
		m_Params.m_SpawnOffsetOS.x = ConfigManager::GetInheritedFloat( sSpawnOffsetOSX, 0.0f, sDefinitionName );
		
		STATICHASH( SpawnOffsetOSY );
		m_Params.m_SpawnOffsetOS.y = ConfigManager::GetInheritedFloat( sSpawnOffsetOSY, 0.0f, sDefinitionName );
		
		STATICHASH( SpawnOffsetOSZ );
		m_Params.m_SpawnOffsetOS.z = ConfigManager::GetInheritedFloat( sSpawnOffsetOSZ, 0.0f, sDefinitionName );

		STATICHASH( SpawnOffsetWSX );
		m_Params.m_SpawnOffsetWS.x = ConfigManager::GetInheritedFloat( sSpawnOffsetWSX, 0.0f, sDefinitionName );

		STATICHASH( SpawnOffsetWSY );
		m_Params.m_SpawnOffsetWS.y = ConfigManager::GetInheritedFloat( sSpawnOffsetWSY, 0.0f, sDefinitionName );

		STATICHASH( SpawnOffsetWSZ );
		m_Params.m_SpawnOffsetWS.z = ConfigManager::GetInheritedFloat( sSpawnOffsetWSZ, 0.0f, sDefinitionName );

		STATICHASH( SpawnExtentsX );
		m_Params.m_SpawnExtents.x = ConfigManager::GetInheritedFloat( sSpawnExtentsX, 0.0f, sDefinitionName );
		
		STATICHASH( SpawnExtentsY );
		m_Params.m_SpawnExtents.y = ConfigManager::GetInheritedFloat( sSpawnExtentsY, 0.0f, sDefinitionName );
		
		STATICHASH( SpawnExtentsZ );
		m_Params.m_SpawnExtents.z = ConfigManager::GetInheritedFloat( sSpawnExtentsZ, 0.0f, sDefinitionName );
		
		STATICHASH( InitialVelocityOSMinX );
		m_Params.m_InitialVelocityOSMin.x = ConfigManager::GetInheritedFloat( sInitialVelocityOSMinX, 0.0f, sDefinitionName );
		
		STATICHASH( InitialVelocityOSMinY );
		m_Params.m_InitialVelocityOSMin.y = ConfigManager::GetInheritedFloat( sInitialVelocityOSMinY, 0.0f, sDefinitionName );
		
		STATICHASH( InitialVelocityOSMinZ );
		m_Params.m_InitialVelocityOSMin.z = ConfigManager::GetInheritedFloat( sInitialVelocityOSMinZ, 0.0f, sDefinitionName );
		
		STATICHASH( InitialVelocityOSMaxX );
		m_Params.m_InitialVelocityOSMax.x = ConfigManager::GetInheritedFloat( sInitialVelocityOSMaxX, 0.0f, sDefinitionName );
		
		STATICHASH( InitialVelocityOSMaxY );
		m_Params.m_InitialVelocityOSMax.y = ConfigManager::GetInheritedFloat( sInitialVelocityOSMaxY, 0.0f, sDefinitionName );
		
		STATICHASH( InitialVelocityOSMaxZ );
		m_Params.m_InitialVelocityOSMax.z = ConfigManager::GetInheritedFloat( sInitialVelocityOSMaxZ, 0.0f, sDefinitionName );
		
		STATICHASH( Elasticity );
		m_Params.m_Elasticity = ConfigManager::GetInheritedFloat( sElasticity, 0.0f, sDefinitionName );
		
		STATICHASH( LifetimeMin );
		m_Params.m_LifetimeMin = ConfigManager::GetInheritedFloat( sLifetimeMin, 0.0f, sDefinitionName );
		
		STATICHASH( LifetimeMax );
		m_Params.m_LifetimeMax = ConfigManager::GetInheritedFloat( sLifetimeMax, 0.0f, sDefinitionName );
		
		STATICHASH( AccelerationWSX );
		m_Params.m_AccelerationWS.x = ConfigManager::GetInheritedFloat( sAccelerationWSX, 0.0f, sDefinitionName );
		
		STATICHASH( AccelerationWSY );
		m_Params.m_AccelerationWS.y = ConfigManager::GetInheritedFloat( sAccelerationWSY, 0.0f, sDefinitionName );
		
		STATICHASH( AccelerationWSZ );
		m_Params.m_AccelerationWS.z = ConfigManager::GetInheritedFloat( sAccelerationWSZ, 0.0f, sDefinitionName );
		
		STATICHASH( ViewBoundRadius );
		m_Params.m_ViewBoundRadius = ConfigManager::GetInheritedFloat( sViewBoundRadius, 1.0f, sDefinitionName );
		
		STATICHASH( InitialSizeMin );
		m_Params.m_InitialSizeMin = ConfigManager::GetInheritedFloat( sInitialSizeMin, 1.0f, sDefinitionName );
		
		STATICHASH( InitialSizeMax );
		m_Params.m_InitialSizeMax = ConfigManager::GetInheritedFloat( sInitialSizeMax, 1.0f, sDefinitionName );
		
		STATICHASH( SizeVelocityMin );
		m_Params.m_SizeVelocityMin = ConfigManager::GetInheritedFloat( sSizeVelocityMin, 0.0f, sDefinitionName );
		
		STATICHASH( SizeVelocityMax );
		m_Params.m_SizeVelocityMax = ConfigManager::GetInheritedFloat( sSizeVelocityMax, 0.0f, sDefinitionName );

		STATICHASH( LinkedRGB );
		m_Params.m_LinkedRGB = ConfigManager::GetInheritedBool( sLinkedRGB, false, sDefinitionName );
		
		STATICHASH( ColorMinR );
		m_Params.m_ColorMin.x = ConfigManager::GetInheritedFloat( sColorMinR, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMinG );
		m_Params.m_ColorMin.y = ConfigManager::GetInheritedFloat( sColorMinG, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMinB );
		m_Params.m_ColorMin.z = ConfigManager::GetInheritedFloat( sColorMinB, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMinA );
		m_Params.m_ColorMin.w = ConfigManager::GetInheritedFloat( sColorMinA, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMaxR );
		m_Params.m_ColorMax.x = ConfigManager::GetInheritedFloat( sColorMaxR, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMaxG );
		m_Params.m_ColorMax.y = ConfigManager::GetInheritedFloat( sColorMaxG, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMaxB );
		m_Params.m_ColorMax.z = ConfigManager::GetInheritedFloat( sColorMaxB, 1.0f, sDefinitionName );
		
		STATICHASH( ColorMaxA );
		m_Params.m_ColorMax.w = ConfigManager::GetInheritedFloat( sColorMaxA, 1.0f, sDefinitionName );
		
		STATICHASH( InitialTrace );
		m_Params.m_InitialTrace = ConfigManager::GetInheritedBool( sInitialTrace, false, sDefinitionName );
		
		STATICHASH( InitialTraceOffsetOSX );
		m_Params.m_InitialTraceOffsetOS.x = ConfigManager::GetInheritedFloat( sInitialTraceOffsetOSX, 0.0f, sDefinitionName );
		
		STATICHASH( InitialTraceOffsetOSY );
		m_Params.m_InitialTraceOffsetOS.y = ConfigManager::GetInheritedFloat( sInitialTraceOffsetOSY, 0.0f, sDefinitionName );
		
		STATICHASH( InitialTraceOffsetOSZ );
		m_Params.m_InitialTraceOffsetOS.z = ConfigManager::GetInheritedFloat( sInitialTraceOffsetOSZ, 0.0f, sDefinitionName );

#if BUILD_DEV
		// Validate data
		const float UpperBound		= m_Params.m_LifetimeMax * m_Params.m_SpawnRate;
		const float MaxParticles	= static_cast<float>( m_Params.m_MaxParticles );
		if( MaxParticles < UpperBound )
		{
			WARNDESC( "Particle system has MaxParticles less than LifetimeMax * SpawnRate. System may starve." );
		}
#endif

		sm_ParamCache.Insert( HashedDefinitionName, m_Params );
	}
}

/*static*/ void EldritchParticles::FlushConfigCache()
{
	sm_ParamCache.Clear();
}

void EldritchParticles::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	GetCachedConfig( DefinitionName );

	IRenderer* const pRenderer = EldritchFramework::GetInstance()->GetRenderer();
	m_TextureMap = pRenderer->GetTextureManager()->GetTexture( m_Params.m_TextureMapName );

	// Make the particle system die out after the last particle
	if( SystemIsFinite() )
	{
		m_ExpireTime = GetTime() + m_Params.m_SystemLifetime;
		m_FinishTime = m_ExpireTime + m_Params.m_LifetimeMax;
	}

	// If this will use an array instead of a list, reserve the space now
	if( IsUnsorted() )
	{
		m_ParticlesArray.Reserve( m_Params.m_MaxParticles );
		m_ParticlesArray.SetDeflate( false );
	}

	CreateMesh();
}

void EldritchParticles::Tick( const float DeltaTime )
{
	//PROFILE_FUNCTION;

	bool ShouldImmediateSpawnMax = m_Params.m_ImmediateSpawnMax && !m_HasImmediateSpawnedMax;

	if( ShouldImmediateSpawnMax || ( !IsExpired() && m_RenderedLastFrame ) )
	{
		uint NumParticlesToSpawn = 0;

		if( ShouldImmediateSpawnMax )
		{
			NumParticlesToSpawn = m_Params.m_MaxParticles;
			m_HasImmediateSpawnedMax = true;
		}
		else if( m_Params.m_SpawnRate > 0.0f )
		{
			m_SpawnAccumulator += DeltaTime;
			NumParticlesToSpawn = (uint)( m_SpawnAccumulator * m_Params.m_SpawnRate );
			m_SpawnAccumulator = Mod( m_SpawnAccumulator, m_Params.m_InvSpawnRate );
		}

		// This logic means that new particles will not be spawned if the limit
		// is exceeded (rather than new particles killing off older ones).
		NumParticlesToSpawn = Min( NumParticlesToSpawn, m_Params.m_MaxParticles - GetNumParticles() );

		SpawnParticles( NumParticlesToSpawn );
	}

	if( m_RenderedLastFrame )
	{
		TickParticles( DeltaTime );
	}

	m_RenderedLastFrame = false;
}

void EldritchParticles::SpawnParticles( uint NumParticlesToSpawn )
{
	XTRACE_FUNCTION;

	for( uint i = 0; i < NumParticlesToSpawn; ++i )
	{
		const Matrix RotationMatrix = m_SystemOrientation.ToMatrix();

		SParticle Particle;
		Particle.m_Lifetime				= Math::Random( m_Params.m_LifetimeMin, m_Params.m_LifetimeMax );
		Particle.m_Size					= Math::Random( m_Params.m_InitialSizeMin, m_Params.m_InitialSizeMax );
		Particle.m_SizeVelocity			= Math::Random( m_Params.m_SizeVelocityMin, m_Params.m_SizeVelocityMax );
		Particle.m_Color				= Math::Random( m_Params.m_ColorMin, m_Params.m_ColorMax );
		Particle.m_InitialOffset		= GetParticleSpawnLocationOffset();
		Particle.m_Location				= m_SystemLocation + Particle.m_InitialOffset;
		Particle.m_Velocity				= Math::Random( m_Params.m_InitialVelocityOSMin, m_Params.m_InitialVelocityOSMax ) * RotationMatrix;
		Particle.m_ViewDistanceSq		= GetViewDistanceSq( Particle.m_Location );

		if( m_Params.m_LinkedRGB )
		{
			Particle.m_Color.z = Particle.m_Color.y = Particle.m_Color.x;
		}

		if( m_Params.m_InitialTrace )
		{
			const Vector OffsetLocation = Particle.m_Location + m_Params.m_InitialTraceOffsetOS * RotationMatrix;

			EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
			const Segment TraceSegment = Segment( Particle.m_Location, OffsetLocation );
			CollisionInfo Info;
			Info.m_CollideWorld = true;

			if( pWorld->Trace( TraceSegment, Info ) )
			{
				Particle.m_Location = Info.m_Intersection;
			}
			else
			{
				Particle.m_Location = OffsetLocation;
			}
		}

		if( IsUnsorted() )
		{
			XTRACE_NAMED( SpawnParticle_Unsorted );
			m_ParticlesArray.PushBack( Particle );
		}
		else
		{
			XTRACE_NAMED( SpawnParticle_Sorted );
			m_ParticlesList.PushBack( Particle );
		}
	}
}

Vector EldritchParticles::GetParticleSpawnLocationOffset() const
{
	const Matrix SystemOrientation	= m_SystemOrientation.ToMatrix();
	const Vector SpawnOffsetOS		= m_Params.m_SpawnOffsetOS * SystemOrientation;
	const Vector SpawnExtents		= Math::Random( -m_Params.m_SpawnExtents, m_Params.m_SpawnExtents );
	const Vector SpawnExtentsOS		= SpawnExtents * SystemOrientation;
	return m_Params.m_SpawnOffsetWS + SpawnOffsetOS + SpawnExtentsOS;
}

void EldritchParticles::TickParticles( const float DeltaTime )
{
	if( IsUnsorted() )
	{
		for( uint ParticleIndex = 0; ParticleIndex < m_ParticlesArray.Size(); )
		{
			SParticle& Particle = m_ParticlesArray[ ParticleIndex ];

			if( TickParticle( Particle, DeltaTime ) )
			{
				++ParticleIndex;
			}
			else
			{
				m_ParticlesArray.FastRemove( ParticleIndex );
			}
		}
	}
	else
	{
		FOR_EACH_LIST_NOINCR( ParticleIter, m_ParticlesList, EldritchParticles::SParticle )
		{
			SParticle& Particle = *ParticleIter;

			if( TickParticle( Particle, DeltaTime ) )
			{
				++ParticleIter;
			}
			else
			{
				m_ParticlesList.Pop( ParticleIter );
			}
		}
	}
}

bool EldritchParticles::TickParticle( SParticle& Particle, float DeltaTime )
{
	if( Particle.m_Lifetime > 0.0f )
	{
		Particle.m_Lifetime -= DeltaTime;
		if( Particle.m_Lifetime <= 0.0f )
		{
			return false;
		}
	}

	const Vector TotalAcceleration = m_Params.m_AccelerationWS;

	Particle.m_Size = Max( 0.0f, Particle.m_Size + Particle.m_SizeVelocity * DeltaTime );
	Particle.m_Velocity += TotalAcceleration * DeltaTime;	// Integrating velocity before position is Euler-Cromer, stabler than Euler

	Vector Offset = Particle.m_Velocity * DeltaTime;
	if( m_Params.m_Collision )
	{
		EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
		const Segment TraceSegment = Segment( Particle.m_Location, Particle.m_Location + Offset );
		CollisionInfo Info;
		Info.m_CollideWorld = true;

		if( pWorld->Trace( TraceSegment, Info ) )
		{
			Offset = Info.m_Plane.ProjectVector( Offset );

			// Collision response--reflect velocity for bounce
			Vector NormalVelocity = Particle.m_Velocity.ProjectionOnto( Info.m_Plane.m_Normal );
			Vector TangentialVelocity = Info.m_Plane.ProjectVector( Particle.m_Velocity );
			Particle.m_Velocity = TangentialVelocity + ( m_Params.m_Elasticity * -NormalVelocity );
		}
	}
	Particle.m_Location += Offset;

	Particle.m_ViewDistanceSq = GetViewDistanceSq( Particle.m_Location );

	return true;
}

void EldritchParticles::Render()
{
	XTRACE_FUNCTION;

	//PROFILE_FUNCTION;

	UpdateMesh();

	if( m_DynamicMesh )
	{
		EldritchWorld* const pWorld = EldritchFramework::GetInstance()->GetWorld();
		m_DynamicMesh->SetIrradianceCube( pWorld->GetIrradianceAt( m_SystemLocation ) );

		if( !m_DynamicMesh->GetTexture( 1 ) )
		{
			EldritchGame* const pGame = EldritchFramework::GetInstance()->GetGame();
			ITexture* const pFogTexture = pGame->GetFogTexture();
			m_DynamicMesh->SetTexture( 1, pFogTexture );
		}

		IRenderer* const pRenderer = EldritchFramework::GetInstance()->GetRenderer();
		pRenderer->AddMesh( m_DynamicMesh );
	}

	m_RenderedLastFrame = true;
}

void EldritchParticles::CreateMesh()
{
	//PROFILE_FUNCTION;

	if( m_Params.m_MaxParticles == 0 )
	{
		m_DynamicMesh = NULL;
	}
	else
	{
		IRenderer* const pRenderer = EldritchFramework::GetInstance()->GetRenderer();

		uint NumVertices	= m_Params.m_MaxParticles * 4;
		uint NumIndices		= m_Params.m_MaxParticles * 6;

		m_VB_Positions.ResizeZero( NumVertices );
		m_VB_FloatColors.ResizeZero( NumVertices );
		m_VB_UVs.ResizeZero( NumVertices );
		m_VB_Normals.ResizeZero( NumVertices );
		m_VB_Indices.ResizeZero( NumIndices );

		// Set up buffers wot won't ever change
		for( uint i = 0; i < m_Params.m_MaxParticles; ++i )
		{
			m_VB_UVs[ i * 4 ] = Vector2( 0.0f, 0.0f );
			m_VB_UVs[ i * 4 + 1 ] = Vector2( 1.0f, 0.0f );
			m_VB_UVs[ i * 4 + 2 ] = Vector2( 0.0f, 1.0f );
			m_VB_UVs[ i * 4 + 3 ] = Vector2( 1.0f, 1.0f );

			m_VB_Indices[ i * 6 ] = (index_t)( i * 4 );
			m_VB_Indices[ i * 6 + 1 ] = (index_t)( i * 4 + 2 );
			m_VB_Indices[ i * 6 + 2 ] = (index_t)( i * 4 + 1 );
			m_VB_Indices[ i * 6 + 3 ] = (index_t)( i * 4 + 2 );
			m_VB_Indices[ i * 6 + 4 ] = (index_t)( i * 4 + 3 );
			m_VB_Indices[ i * 6 + 5 ] = (index_t)( i * 4 + 1 );
		}

		IVertexBuffer* VertexBuffer = pRenderer->CreateVertexBuffer();
		InitVertexBuffers( VertexBuffer );

		// Register a callback so we can rebuild this mesh when device is reset
		SDeviceResetCallback Callback;
		Callback.m_Callback = DeviceResetCallback;
		Callback.m_Void = this;
		VertexBuffer->RegisterDeviceResetCallback( Callback );

		pRenderer->AddDynamicVertexBuffer( VertexBuffer );

		uint VertexSignature = VD_POSITIONS | VD_UVS | VD_FLOATCOLORS_SM2 | VD_NORMALS;
		IVertexDeclaration* VertexDeclaration = pRenderer->GetVertexDeclaration( VertexSignature );

		IIndexBuffer* const IndexBuffer = pRenderer->CreateIndexBuffer();
		IndexBuffer->Init( NumIndices, m_VB_Indices.GetData() );
		IndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

		m_DynamicMesh = new EldritchMesh;
		ASSERT( m_DynamicMesh );

		m_DynamicMesh->Initialize( VertexBuffer, VertexDeclaration, IndexBuffer, NULL );
		m_DynamicMesh->SetTexture( 0, m_TextureMap );

#if BUILD_DEBUG
		m_DynamicMesh->m_Name = "Particles";
#endif

		if( m_Params.m_AdditiveDraw )
		{
			m_DynamicMesh->SetMaterialDefinition( "Material_ParticlesAdditive", pRenderer );
		}
		else if( m_Params.m_AlphaTestDraw )
		{
			m_DynamicMesh->SetMaterialDefinition( "Material_ParticlesAlphaTest", pRenderer );
		}
		else
		{
			m_DynamicMesh->SetMaterialDefinition( "Material_ParticlesAlpha", pRenderer );
		}

		uint MaterialFlags = MAT_DYNAMIC;
		MaterialFlags |= m_Params.m_AlphaTestDraw ? MAT_NONE : MAT_ALPHA;
		MaterialFlags |= m_Params.m_ForegroundDraw ? MAT_FOREGROUND : MAT_WORLD;
		MaterialFlags |= m_Params.m_AlwaysDraw ? MAT_ALWAYS : MAT_NONE;
		m_DynamicMesh->SetMaterialFlags( MaterialFlags );
	}
}

inline uint EldritchParticles::GetNumParticles() const
{
	return IsUnsorted() ? m_ParticlesArray.Size() : m_ParticlesList.Size();
}

void EldritchParticles::InitVertexBuffers( IVertexBuffer* pVertexBuffer )
{
	//PROFILE_FUNCTION;

	ASSERT( pVertexBuffer );
	ASSERT( m_Params.m_MaxParticles > 0 );
	ASSERT( m_VB_Positions.GetData() );
	ASSERT( m_VB_FloatColors.GetData() );
	ASSERT( m_VB_UVs.GetData() );
	ASSERT( m_VB_Normals.GetData() );

	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= m_Params.m_MaxParticles * 4;
	InitStruct.Positions	= m_VB_Positions.GetData();
	InitStruct.FloatColors1 = m_VB_FloatColors.GetData();
	InitStruct.UVs			= m_VB_UVs.GetData();
	InitStruct.Normals		= m_VB_Normals.GetData();
	InitStruct.Dynamic = true;
	pVertexBuffer->Init( InitStruct );

	pVertexBuffer->SetNumVertices( GetNumParticles() * 4 );
}

void EldritchParticles::UpdateMesh()
{
	XTRACE_FUNCTION;

	//PROFILE_FUNCTION;

	if( m_Params.m_MaxParticles == 0 )
	{
		return;
	}

	DEVASSERT( m_DynamicMesh );
	if( !m_DynamicMesh )
	{
		return;
	}

	uint NumParticles = GetNumParticles();
	DEVASSERT( NumParticles <= m_Params.m_MaxParticles );

	// Sort by view distance (requires that I keep particles as a linked list).
	// Additive and alpha-tested particles draw the same regardless of order, so no need to sort them.
	if( !IsUnsorted() )
	{
		m_ParticlesList.Sort();
	}

	uint NumVertices	= NumParticles * 4;
	uint NumIndices		= NumParticles * 6;

	SParticleMeshParams MeshParams;

	View* const pView	= EldritchFramework::GetInstance()->GetMainView();
	DEVASSERT( pView );

	// Zero roll so that particles do not roll with player's view--that looks especially silly on foliage.
	Angles ViewAngle	= pView->m_Rotation;
	ViewAngle.Roll		= 0.0f;

	MeshParams.m_ViewLocation	= pView->m_Location;
	MeshParams.m_ViewAxis		= ViewAngle.ToVector();
	MeshParams.m_AngleMatrix	= ViewAngle.ToMatrix();
	MeshParams.m_OriginMatrix	= Matrix::CreateTranslation( -m_SystemLocation );

	if( IsUnsorted() )
	{
		XTRACE_NAMED( AddParticlesToMesh_Unsorted );

		for( uint ParticleIndex = 0; ParticleIndex < NumParticles; ++ParticleIndex )
		{
			EldritchParticles::SParticle& Particle = m_ParticlesArray[ ParticleIndex ];
			AddParticleToMesh( ParticleIndex, Particle, MeshParams );
		}
	}
	else
	{
		XTRACE_NAMED( AddParticlesToMesh_Sorted );

		uint ParticleIndex = 0;
		FOR_EACH_LIST( ParticleIter, m_ParticlesList, EldritchParticles::SParticle )
		{
			EldritchParticles::SParticle& Particle = ParticleIter.GetValue();
			AddParticleToMesh( ParticleIndex, Particle, MeshParams );
			++ParticleIndex;
		}
	}

	// Copy updated local vertex elements into vertex buffers;
	// it'd be faster to not do memcpy here, but I do still need to maintain
	// the local copies for rebuilding on device reset, so...
	XTRACE_BEGIN( UpdateBuffers );
		IVertexBuffer* const	pVertexBuffer	= m_DynamicMesh->m_VertexBuffer;
		IIndexBuffer* const		pIndexBuffer	= m_DynamicMesh->m_IndexBuffer;

		DEVASSERT( pVertexBuffer );
		DEVASSERT( pIndexBuffer );

		Vector* const pPositions = static_cast<Vector*>( pVertexBuffer->Lock( IVertexBuffer::EVE_Positions ) );
		DEVASSERT( pPositions );
		Vector4* const pFloatColors = static_cast<Vector4*>( pVertexBuffer->Lock( IVertexBuffer::EVE_FloatColors1 ) );
		DEVASSERT( pFloatColors );
		Vector* const pNormals = static_cast<Vector*>( pVertexBuffer->Lock( IVertexBuffer::EVE_Normals ) );
		DEVASSERT( pNormals );

		DEVASSERT( m_VB_Positions.GetData() );
		DEVASSERT( m_VB_FloatColors.GetData() );
		DEVASSERT( m_VB_Normals.GetData() );

		// Eh, why not leave this conditional here. It can't hurt.
		if( pPositions && pFloatColors && pNormals )
		{
			memcpy( pPositions,		m_VB_Positions.GetData(),	NumVertices * sizeof( Vector ) );
			memcpy( pFloatColors,	m_VB_FloatColors.GetData(),	NumVertices * sizeof( Vector4 ) );
			memcpy( pNormals,		m_VB_Normals.GetData(),		NumVertices * sizeof( Vector ) );
		}

		pVertexBuffer->Unlock( IVertexBuffer::EVE_Positions );
		pVertexBuffer->Unlock( IVertexBuffer::EVE_FloatColors1 );
		pVertexBuffer->Unlock( IVertexBuffer::EVE_Normals );

		pVertexBuffer->SetNumVertices( NumVertices );
		pIndexBuffer->SetNumIndices( NumIndices );
	XTRACE_END;

	// Set mesh location so we can sort meshes and look up irradiance volume
	m_DynamicMesh->m_Location = m_SystemLocation;

	// Cheap bounding box; it's not worthwhile to compute a real bound every frame
	Vector BoundRadius( m_Params.m_ViewBoundRadius, m_Params.m_ViewBoundRadius, m_Params.m_ViewBoundRadius );
	m_DynamicMesh->m_AABB = AABB( m_SystemLocation - BoundRadius, m_SystemLocation + BoundRadius );
}

inline void EldritchParticles::AddParticleToMesh( const uint Index, const SParticle& Particle, const SParticleMeshParams& MeshParams )
{
	uint j = Index * 4;
	uint k = j + 1;
	uint l = k + 1;
	uint m = l + 1;

	const Matrix& ParticleRotationMatrix = MeshParams.m_AngleMatrix;
	const Matrix ParticleMatrix = ParticleRotationMatrix * Matrix::CreateTranslation( Particle.m_Location ) * MeshParams.m_OriginMatrix;	// Translate particles to be relative to origin (then set mesh location)

	const float HalfSize = Particle.m_Size * 0.5f;
	m_VB_Positions[ j ] = Vector( -HalfSize, 0.0f, HalfSize ) * ParticleMatrix;
	m_VB_Positions[ k ] = Vector( HalfSize, 0.0f, HalfSize ) * ParticleMatrix;
	m_VB_Positions[ l ] = Vector( -HalfSize, 0.0f, -HalfSize ) * ParticleMatrix;
	m_VB_Positions[ m ] = Vector( HalfSize, 0.0f, -HalfSize ) * ParticleMatrix;

	const Vector4& FloatColor = Particle.m_Color;

	m_VB_FloatColors[ j ] = FloatColor;
	m_VB_FloatColors[ k ] = FloatColor;
	m_VB_FloatColors[ l ] = FloatColor;
	m_VB_FloatColors[ m ] = FloatColor;

	static const Vector kNormalTopLeft(		-0.577349f,	-0.577349f,	0.577349f );
	static const Vector kNormalTopRight(	0.577349f,	-0.577349f,	0.577349f );
	static const Vector kNormalBottomLeft(	-0.577349f,	-0.577349f,	-0.577349f );
	static const Vector kNormalBottomRight(	0.577349f,	-0.577349f,	-0.577349f );

	// Angle normals out to give particles some shape
	m_VB_Normals[ j ] = kNormalTopLeft		* ParticleRotationMatrix;
	m_VB_Normals[ k ] = kNormalTopRight		* ParticleRotationMatrix;
	m_VB_Normals[ l ] = kNormalBottomLeft	* ParticleRotationMatrix;
	m_VB_Normals[ m ] = kNormalBottomRight	* ParticleRotationMatrix;
}

float EldritchParticles::GetViewDistanceSq( const Vector& ParticleLocation )
{
	View* const pView = EldritchFramework::GetInstance()->GetMainView();
	return ( pView->m_Location - ParticleLocation ).LengthSquared();
}

/*static*/ void EldritchParticles::DeviceResetCallback( void* pVoid, IVertexBuffer* pBuffer )
{
	EldritchParticles* const pParticleSystem = static_cast<EldritchParticles*>( pVoid );
	pParticleSystem->InitVertexBuffers( pBuffer );
}

inline bool EldritchParticles::IsUnsorted() const
{
	return m_Params.m_AdditiveDraw || m_Params.m_AlphaTestDraw;
}

inline float EldritchParticles::GetTime() const
{
	return WBWorld::GetInstance()->GetTime();
}

inline bool EldritchParticles::SystemIsFinite() const
{
	return m_Params.m_SystemLifetime > 0.0f || m_Params.m_SpawnRate == 0.0f;
}

inline bool EldritchParticles::IsExpired() const
{
	return SystemIsFinite() && GetTime() >= m_ExpireTime;
}

bool EldritchParticles::IsFinished() const
{
	return SystemIsFinite() && GetTime() >= m_FinishTime;
}