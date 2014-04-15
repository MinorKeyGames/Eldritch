#include "core.h"
#include "mesh.h"
#include "bonearray.h"
#include "ivertexdeclaration.h"
#include "configmanager.h"
#include "ibonemodifier.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "mathcore.h"

Mesh::Mesh( IVertexBuffer* pVertexBuffer, IVertexDeclaration* pVertexDeclaration, IIndexBuffer* pIndexBuffer, BoneArray* pBones )
:	m_VertexBuffer( pVertexBuffer )
,	m_VertexDeclaration( pVertexDeclaration )
,	m_IndexBuffer( pIndexBuffer )
,	m_Bones( pBones )
,	m_Material()
,	m_MutableVisible( true )
,	m_Location()
,	m_Scale( 1.0f, 1.0f, 1.0f )
,	m_Rotation()
,	m_BoneMatrices( NULL )
,	m_DirtyBoneMatrices( false )
,	m_BoneModifiers()
,	m_AABB()
,	m_CollisionMesh()
,	m_AnimationState()
,	m_IrradianceVolume()
,	m_ConstantColor( 1.0f, 1.0f, 1.0f, 1.0f )
#if BUILD_DEV
,	m_IsDebugMesh( false )
#endif
#if BUILD_DEBUG
,	m_Name( "" )
#endif
{
	m_AnimationState.SetMesh( this );
	Initialize( pVertexBuffer, pVertexDeclaration, pIndexBuffer, pBones );
}

Mesh::~Mesh()
{
	SafeRelease( m_VertexBuffer );
	SafeRelease( m_IndexBuffer );
	SafeRelease( m_Bones );
	SafeDeleteArray( m_BoneMatrices );
	m_BoneModifiers.Clear();
}

void Mesh::Initialize( IVertexBuffer* pVertexBuffer, IVertexDeclaration* pVertexDeclaration, IIndexBuffer* pIndexBuffer, BoneArray* pBones )
{
	m_VertexBuffer = pVertexBuffer;
	if( m_VertexBuffer )
	{
		m_VertexBuffer->AddReference();
	}

	m_VertexDeclaration = pVertexDeclaration;

	m_IndexBuffer = pIndexBuffer;
	if( m_IndexBuffer )
	{
		m_IndexBuffer->AddReference();
	}

	m_Bones = pBones;
	if( m_Bones )
	{
		m_Bones->AddReference();
	}
}

void Mesh::Reinitialize( IVertexBuffer* pVertexBuffer, IVertexDeclaration* pVertexDeclaration, IIndexBuffer* pIndexBuffer, BoneArray* pBones )
{
	if( m_VertexBuffer != pVertexBuffer )
	{
		SafeRelease( m_VertexBuffer );
		m_VertexBuffer = pVertexBuffer;
		if( m_VertexBuffer )
		{
			m_VertexBuffer->AddReference();
		}
	}

	m_VertexDeclaration = pVertexDeclaration;

	if( m_IndexBuffer != pIndexBuffer )
	{
		SafeRelease( m_IndexBuffer );
		m_IndexBuffer = pIndexBuffer;
		if( m_IndexBuffer )
		{
			m_IndexBuffer->AddReference();
		}
	}

	if( m_Bones != pBones )
	{
		SafeRelease( m_Bones );
		m_Bones = pBones;
		if( m_Bones )
		{
			m_Bones->AddReference();
		}
	}
}

void Mesh::Reinitialize( Mesh* pMesh )
{
	if( pMesh )
	{
		Reinitialize( pMesh->m_VertexBuffer, pMesh->m_VertexDeclaration, pMesh->m_IndexBuffer, pMesh->m_Bones );
	}
}

void Mesh::SetVertexDeclaration( IVertexDeclaration* const pVertexDeclaration )
{
	m_VertexDeclaration = pVertexDeclaration;
}

ITexture* Mesh::GetTexture( unsigned int Stage ) const
{
	DEVASSERT( Stage < MAX_TEXTURE_STAGES );
	return m_Material.GetTexture( Stage );
}

void Mesh::SetTexture( unsigned int Stage, ITexture* Texture )
{
	DEVASSERT( Stage < MAX_TEXTURE_STAGES );
	m_Material.SetTexture( Stage, Texture );
}

IShaderProgram* Mesh::GetShaderProgram() const
{
	return m_Material.GetShaderProgram();
}

void Mesh::SetMaterialDefinition( const SimpleString& DefinitionName, IRenderer* const pRenderer )
{
	ASSERT( m_VertexDeclaration );
	m_Material.SetDefinition( DefinitionName, pRenderer, m_VertexDeclaration );
}

uint Mesh::GetMaterialFlags() const
{
	return m_Material.GetFlags();
}

void Mesh::SetMaterialFlags( unsigned int Flags, unsigned int Mask /*= MAT_ALL*/ )
{
	m_Material.SetFlags( Flags, Mask );
}

void Mesh::SetMaterialFlag( unsigned int Flag, bool Set )
{
	m_Material.SetFlag( Flag, Set );
}

Matrix Mesh::GetConcatenatedTransforms()
{
	return
		Matrix::CreateScale( m_Scale ) *
		m_Rotation.ToMatrix() *
		Matrix::CreateTranslation( m_Location );
}

bool Mesh::IsAnimated() const
{
	return ( m_Bones != NULL );
}

void Mesh::Tick( float DeltaTime )
{
	if( IsAnimated() )
	{
		m_DirtyBoneMatrices = true;
		m_AnimationState.Tick( DeltaTime );
	}
}

void Mesh::CopyAnimationsFrom( Mesh* const pMesh )
{
	DEVASSERT( pMesh );
	DEVASSERT( pMesh->m_Bones );

	if( m_Bones != pMesh->m_Bones )
	{
		SafeRelease( m_Bones );
		m_Bones = pMesh->m_Bones;
		m_Bones->AddReference();
	}
}

void Mesh::SuppressAnimEvents( const bool Suppress )
{
	m_AnimationState.SuppressAnimEvents( Suppress );
}

void Mesh::PlayAnimation( const HashedString& AnimationName, AnimationState::SPlayAnimationParams& PlayParams )
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		PlayParams.m_SuppressAnimEvents = false;
		m_AnimationState.PlayAnimation( m_Bones->GetAnimation( AnimationName ), PlayParams );
	}
}

void Mesh::SetAnimation( int AnimationIndex, AnimationState::SPlayAnimationParams& PlayParams )
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		PlayParams.m_SuppressAnimEvents = true;
		m_AnimationState.PlayAnimation( m_Bones->GetAnimation( AnimationIndex ), PlayParams );
	}
}

void Mesh::StopAnimation()
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		m_AnimationState.StopAnimation();
	}
}

const Animation* Mesh::GetPlayingAnimation() const
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		return m_AnimationState.GetPlayingAnimation();
	}
	else
	{
		return NULL;
	}
}

Animation* Mesh::GetAnimation( const SimpleString& Name ) const
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		return m_Bones->GetAnimation( Name );
	}

	return NULL;
}

void Mesh::AddAnimationListener( const SAnimationListener& AnimationListener )
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		m_AnimationState.AddAnimationListener( AnimationListener );
	}
}

void Mesh::RemoveAnimationListener( const SAnimationListener& AnimationListener )
{
	DEVASSERT( IsAnimated() );

	if( IsAnimated() )
	{
		m_AnimationState.RemoveAnimationListener( AnimationListener );
	}
}

void Mesh::UpdateBones()
{
	DEVASSERT( IsAnimated() );

	// Deferred creation of bone matrices
	if( !m_BoneMatrices )
	{
		ASSERT( m_Bones->GetNumBones() <= MAX_BONE_MATRICES );
		m_BoneMatrices		= new Matrix[ m_Bones->GetNumBones() ];
		m_DirtyBoneMatrices	= true;
	}

	if( m_DirtyBoneMatrices )
	{
		m_DirtyBoneMatrices = false;

		m_AnimationState.UpdateBones( m_Bones, m_BoneMatrices );

		for( uint BoneModifierIndex = 0; BoneModifierIndex < m_BoneModifiers.Size(); ++BoneModifierIndex )
		{
			m_BoneModifiers[ BoneModifierIndex ]->Modify( m_Bones, m_BoneMatrices );
		}
	}
}

void Mesh::AddBoneModifier( IBoneModifier* pBoneModifier )
{
	m_BoneModifiers.PushBack( pBoneModifier );
}

void Mesh::GetAnimationVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity )
{
	DEBUGASSERT( IsAnimated() );
	return m_AnimationState.GetAnimationVelocity( OutVelocity, OutRotationalVelocity );
}

int Mesh::GetAnimationIndex() const
{
	if( m_Bones )
	{
		return m_Bones->GetAnimationIndex( m_AnimationState.GetAnimation() );
	}
	else
	{
		return INVALID_INDEX;
	}
}

float Mesh::GetAnimationTime() const
{
	return m_AnimationState.GetAnimationTime();
}

void Mesh::SetAnimationTime( const float AnimationTime )
{
	m_AnimationState.SetAnimationTime( AnimationTime );
}

float Mesh::GetAnimationPlayRate() const
{
	return m_AnimationState.GetAnimationPlayRate();
}

void Mesh::SetAnimationPlayRate( const float AnimationPlayRate )
{
	m_AnimationState.SetAnimationPlayRate( AnimationPlayRate );
}

AnimationState::EAnimationEndBehavior Mesh::GetAnimationEndBehavior() const
{
	return m_AnimationState.GetAnimationEndBehavior();
}

// Blend over time to discrete volume values (as opposed to interpolating
// between eight near volumes, which would be a lot of calculations and would
// dilute extreme values).
void Mesh::BlendIrradianceVolume( const IrradianceVolumes& Volumes, float DeltaTime, const Vector& Location, const Vector4& ConstantTerm /*= Vector4( 0.0f, 0.0f, 0.0f, 0.0f )*/ )
{
	STATICHASH( IrradianceVolumeBlendRate );
	const SIrradianceVolume& TargetVolume = Volumes.GetNearestVolume( Location );
	float BlendT = Saturate( DeltaTime * ConfigManager::GetFloat( sIrradianceVolumeBlendRate, 1.0f ) );
	for( uint i = 0; i < 6; ++i )
	{
		m_IrradianceVolume.m_Light[i] =
			( m_IrradianceVolume.m_Light[i] * ( 1.0f - BlendT ) ) +
			( ( ConstantTerm + TargetVolume.m_Light[i] ) * BlendT );
	}
}

void Mesh::SetIrradianceVolume( const IrradianceVolumes& Volumes, const Vector& Location, const Vector4& ConstantTerm /*= Vector4( 0.0f, 0.0f, 0.0f, 0.0f )*/ )
{
	const SIrradianceVolume& TargetVolume = Volumes.GetNearestVolume( Location );
	for( uint i = 0; i < 6; ++i )
	{
		m_IrradianceVolume.m_Light[i] = ConstantTerm + TargetVolume.m_Light[i];
	}
}

/*virtual*/ Vector Mesh::GetSortLocation()
{
	return m_Location;
}