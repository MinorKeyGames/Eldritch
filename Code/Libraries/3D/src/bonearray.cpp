#include "core.h"
#include "bonearray.h"
#include "animation.h"
#include "simplestring.h"
#include "hashedstring.h"
#include "configmanager.h"

#include <memory.h>
#include <string.h>

BoneArray::BoneArray()
:	m_RefCount( 0 )
,	m_NumFrames( 0 )
,	m_NumBones( 0 )
,	m_NumAnimations( 0 )
,	m_BoneNames( NULL )
,	m_Bones( NULL )
,	m_Animations( NULL )
{
}

BoneArray::~BoneArray()
{
	SafeDeleteArray( m_BoneNames );
	SafeDeleteArray( m_Bones );
	SafeDeleteArray( m_Animations );
}

int BoneArray::AddReference()
{
	++m_RefCount;
	return m_RefCount;
}

int BoneArray::Release()
{
	--m_RefCount;
	if( m_RefCount <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCount;
}

// Make deep copies so the caller can delete its arrays
// (I chose to do this because it's synonymous with copying
// verts into D3D buffers.)
void BoneArray::Init( HashedString* BoneNames, SBone* Bones, const char* MeshFilename, Animation* Animations, int NumFrames, int NumBones, int NumAnims )
{
	m_NumFrames		= NumFrames;
	m_NumBones		= NumBones;
	m_NumAnimations	= NumAnims;

	int TotalBones	= m_NumFrames * m_NumBones;

	m_BoneNames		= new HashedString[ NumBones ];
	memcpy_s( m_BoneNames, sizeof( HashedString ) * NumBones, BoneNames, sizeof( HashedString ) * NumBones );

	m_Bones			= new SBone[ TotalBones ];
	memcpy_s( m_Bones, sizeof( SBone ) * TotalBones, Bones, sizeof( SBone ) * TotalBones );

	m_Animations	= new Animation[ NumAnims ];
	memcpy_s( m_Animations, sizeof( Animation ) * NumAnims, Animations, sizeof( Animation ) * NumAnims );

	// Set runtime anim properties from config
	STATICHASH( AnimationMap );
	MAKEHASH( MeshFilename );
	SimpleString AnimationMapName = ConfigManager::GetString( sMeshFilename, "", sAnimationMap );
	for( int AnimIndex = 0; AnimIndex < NumAnims; ++AnimIndex )
	{
		Animation& Animation = m_Animations[ AnimIndex ];
		Animation.InitializeFromDefinition( SimpleString::PrintF( "%s:%s", AnimationMapName.CStr(), Animation.m_Name ) );
	}
}

// Frame 0 should be T-pose, and frame numbers should match Blender
SBone* BoneArray::GetBone( int Index, int Frame /*= FRAME_TPOSE*/ ) const
{
	ASSERT( Index >= 0 && Index < m_NumBones );
	ASSERT( Frame < m_NumFrames );
	ASSERT( m_Bones );

	return m_Bones + ( Frame * m_NumBones ) + Index;
}

int BoneArray::GetBoneIndex( const HashedString& Name ) const
{
	ASSERT( m_BoneNames );
	for( int i = 0; i < m_NumBones; ++i )
	{
		if( Name == m_BoneNames[i] )
		{
			return i;
		}
	}
	return INVALID_INDEX;
}

int BoneArray::GetNumFrames() const
{
	return m_NumFrames;
}

int BoneArray::GetNumBones() const
{
	return m_NumBones;
}

int BoneArray::GetNumAnimations() const
{
	return m_NumAnimations;
}

Animation* BoneArray::GetAnimation( int Index ) const
{
	// This case is sometimes occurring in Eldritch due to hand animation swapping.
	// Until I can isolate a cause, I'm making it more graceful.
	if( Index >= m_NumAnimations )
	{
		WARN;
		return NULL;
	}

	ASSERT( m_Animations );

	if( Index != INVALID_INDEX )
	{
		return m_Animations + Index;
	}
	else
	{
		return NULL;
	}
}

Animation* BoneArray::GetAnimation( const HashedString& Name ) const
{
	ASSERT( m_Animations );

	for( int i = 0; i < m_NumAnimations; ++i )
	{
		if( Name == m_Animations[i].m_HashedName )
		{
			return m_Animations + i;
		}
	}
	return NULL;
}

int BoneArray::GetAnimationIndex( const Animation* pAnimation ) const
{
	for( int i = 0; i < m_NumAnimations; ++i )
	{
		if( m_Animations + i == pAnimation )
		{
			return i;
		}
	}
	return INVALID_INDEX;
}