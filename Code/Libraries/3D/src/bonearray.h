#ifndef BONEARRAY_H
#define BONEARRAY_H

#include "vector.h"
#include "quat.h"

#define INVALID_INDEX	-1
#define FRAME_TPOSE		0

struct SBone
{
	Quat			m_Quat;
	Vector			m_Vector;
};

class Animation;

// 1D array, laid out like a 2D array, all bones by all frames (frame-major)
class BoneArray
{
public:
	BoneArray();
	~BoneArray();

	int				AddReference();
	int				Release();

	void			Init( HashedString* BoneNames, SBone* Bones, const char* MeshFilename, Animation* Animations, int NumFrames, int NumBones, int NumAnims );
	SBone*			GetBone( int Index, int Frame = FRAME_TPOSE ) const;
	int				GetBoneIndex( const HashedString& Name ) const;	// Returns the bone index, or INVALID_INDEX if not found

	inline void		GetInterpolatedBone( int Index, int Frame, int NextFrame, float Delta, SBone& OutInterpolatedBone ) const
	{
		ASSERT( Index >= 0 && Index < m_NumBones );
		ASSERT( Frame >= 0 && Frame < m_NumFrames );
		ASSERT( NextFrame >= 0 && NextFrame < m_NumFrames );
		ASSERT( Delta >= 0.f && Delta <= 1.f );
		ASSERT( m_Bones );

		SBone* Bone1 = m_Bones + ( Frame * m_NumBones ) + Index;
		SBone* Bone2 = m_Bones + ( NextFrame * m_NumBones ) + Index;

		GetInterpolatedBone( *Bone1, *Bone2, Delta, OutInterpolatedBone );
	}

	static inline void	GetInterpolatedBone( const SBone& Bone1, const SBone& Bone2, float Delta, SBone& OutInterpolatedBone )
	{
		Quat	InterpRot	= Bone1.m_Quat.SLERP( Delta, Bone2.m_Quat );
		Vector	InterpPos	= Bone1.m_Vector.LERP( Delta, Bone2.m_Vector );

		OutInterpolatedBone.m_Quat = InterpRot;
		OutInterpolatedBone.m_Vector = InterpPos;
	}

	int				GetNumFrames() const;
	int				GetNumBones() const;
	int				GetNumAnimations() const;

	Animation*		GetAnimation( int Index ) const;
	Animation*		GetAnimation( const HashedString& Name ) const;
	int				GetAnimationIndex( const Animation* pAnimation ) const;	// Returns INVALID_INDEX if not found

private:
	int			m_RefCount;

	int			m_NumFrames;
	int			m_NumBones;
	int			m_NumAnimations;

	HashedString*	m_BoneNames;
	SBone*			m_Bones;
	Animation*		m_Animations;
};

#endif // BONEARRAY_H