#ifndef MESH_H
#define MESH_H

#include "material.h"
#include "vector.h"
#include "angles.h"
#include "matrix.h"
#include "3d.h"
#include "aabb.h"
#include "collisionmesh.h"
#include "simplestring.h"
#include "vector4.h"
#include "irradiancevolumes.h"
#include "animationstate.h"
#include "array.h"

class BoneArray;
class IVertexBuffer;
class IIndexBuffer;
class IVertexDeclaration;
class ITexture;
class IBoneModifier;

// This is a very struct-y class, it's all
// public--and I hate classes that just have
// Get*/Set* functions on EVERYTHING, so I
// guess it's okay for now.

class Mesh
{
public:
	// Serves as default constructor
	Mesh(	IVertexBuffer* pVertexBuffer = NULL,
			IVertexDeclaration* pVertexDeclaration = NULL,
			IIndexBuffer* pIndexBuffer = NULL,
			BoneArray* pBones = NULL );
	virtual ~Mesh();

	void			Initialize( IVertexBuffer* pVertexBuffer, IVertexDeclaration* pVertexDeclaration, IIndexBuffer* pIndexBuffer, BoneArray* pBones );

	void			Reinitialize( IVertexBuffer* pVertexBuffer, IVertexDeclaration* pVertexDeclaration, IIndexBuffer* pIndexBuffer, BoneArray* pBones );
	void			Reinitialize( Mesh* pMesh );	// Assigns this mesh's buffers, but no other properties

	void			SetVertexDeclaration( IVertexDeclaration* const pVertexDeclaration );

	Matrix			GetConcatenatedTransforms();

	ITexture*		GetTexture( unsigned int Stage ) const;
	void			SetTexture( unsigned int Stage, ITexture* Texture );

	IShaderProgram*	GetShaderProgram() const;

	const Material&	GetMaterial() const { return m_Material; }
	void			SetMaterialDefinition( const SimpleString& DefinitionName, IRenderer* const pRenderer );

	uint			GetMaterialFlags() const;
	void			SetMaterialFlags( unsigned int Flags, unsigned int Mask = MAT_ALL );
	void			SetMaterialFlag( unsigned int Flag, bool Set );

	bool			IsAnimated() const;
	void			Tick( float DeltaTime );
	void			CopyAnimationsFrom( Mesh* const pMesh );
	void			SuppressAnimEvents( const bool Suppress );
	void			PlayAnimation( const HashedString& AnimationName, AnimationState::SPlayAnimationParams& PlayParams );
	void			SetAnimation( int AnimationIndex, AnimationState::SPlayAnimationParams& PlayParams );
	void			StopAnimation();
	const Animation*	GetPlayingAnimation() const;
	Animation*		GetAnimation( const SimpleString& Name ) const;
	void			AddAnimationListener( const SAnimationListener& AnimationListener );
	void			RemoveAnimationListener( const SAnimationListener& AnimationListener );
	void			UpdateBones();	// This only applies updates if the matrices have been marked dirty, so it can be called as often as needed
	void			AddBoneModifier( IBoneModifier* pBoneModifier );
	void			GetAnimationVelocity( Vector& OutVelocity, Angles& OutRotationalVelocity );

	// Accessors used to serialize animation state
	int				GetAnimationIndex() const;
	float			GetAnimationTime() const;
	void			SetAnimationTime( const float AnimationTime );
	float			GetAnimationPlayRate() const;
	void			SetAnimationPlayRate( const float AnimationPlayRate );
	AnimationState::EAnimationEndBehavior GetAnimationEndBehavior() const;

	// Blend current irradiance volume to a target volume over time
	void			BlendIrradianceVolume( const IrradianceVolumes& Volumes, float DeltaTime, const Vector& Location, const Vector4& ConstantTerm = Vector4( 0.0f, 0.0f, 0.0f, 0.0f ) );
	void			SetIrradianceVolume( const IrradianceVolumes& Volumes, const Vector& Location, const Vector4& ConstantTerm = Vector4( 0.0f, 0.0f, 0.0f, 0.0f ) );

	// Get the location to use when sorting alpha list
	virtual Vector	GetSortLocation();

public:
	IVertexBuffer*		m_VertexBuffer;
	IVertexDeclaration*	m_VertexDeclaration;
	IIndexBuffer*		m_IndexBuffer;
	BoneArray*			m_Bones;

	Material		m_Material;
	bool			m_MutableVisible;

	Vector			m_Location;		// Could just store a world matrix and avoid recomputing
	Vector			m_Scale;		// it every time, but this prevents some matrix drift...
	Angles			m_Rotation;

	Matrix*					m_BoneMatrices;
	bool					m_DirtyBoneMatrices;
	Array< IBoneModifier* >	m_BoneModifiers;

	// TODO: Use different kinds of bounds as needed
	AABB			m_AABB;

	// TODO: Maybe move some of these more specific members into subclasses?

	// Not a pointer because CollisionMesh is such a lightweight class
	CollisionMesh	m_CollisionMesh;

	// Likewise not a pointer because it's lightweight
	AnimationState	m_AnimationState;

	SIrradianceVolume	m_IrradianceVolume;

	Vector4			m_ConstantColor;	// Used for fonts, drop shadows, etc.

#if BUILD_DEV
	bool			m_IsDebugMesh;	// For automatically deleting debug lines and boxes
#endif

#if BUILD_DEBUG
	SimpleString	m_Name;
#endif
};

#endif // MESH_H