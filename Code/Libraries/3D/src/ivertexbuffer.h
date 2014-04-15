#ifndef IVERTEXBUFFER_H
#define IVERTEXBUFFER_H

#include "3d.h"

class Vector;
class Vector4;
class Vector2;
class VertexWeight;
struct SBoneData;
class IVertexBuffer;

typedef void ( *DeviceResetCallback )( void*, IVertexBuffer* );
struct SDeviceResetCallback
{
	DeviceResetCallback	m_Callback;
	void*				m_Void;
};

class IVertexBuffer
{
protected:
	virtual ~IVertexBuffer() {}

public:
	struct SInit
	{
		SInit()
			:	NumVertices(0)
			,	Positions( NULL )
			,	Colors( NULL )
#if USE_HDR
			,	FloatColors1( NULL )
			,	FloatColors2( NULL )
			,	FloatColors3( NULL )
#endif
			,	UVs( NULL )
			,	Normals( NULL )
			,	Tangents( NULL )
			,	BoneIndices( NULL )
			,	BoneWeights( NULL )
			,	Dynamic( false ) {}

		uint		NumVertices;
		Vector*		Positions;
		uint*		Colors;
#if USE_HDR
		Vector4*	FloatColors1;
		Vector4*	FloatColors2;
		Vector4*	FloatColors3;
#endif
		Vector2*	UVs;
		Vector*		Normals;
		Vector4*	Tangents;
		SBoneData*	BoneIndices;
		SBoneData*	BoneWeights;
		bool		Dynamic;
	};

	// Just using these for locking right now
	enum EVertexElements
	{
		EVE_Positions,
		EVE_Colors,
#if USE_HDR
		EVE_FloatColors1,
		EVE_FloatColors2,
		EVE_FloatColors3,
#endif
		EVE_UVs,
		EVE_Normals,
		EVE_Tangents,
		EVE_BoneIndices,
		EVE_BoneWeights,
	};

	virtual void		Init( const SInit& InitStruct ) = 0;

	virtual void*		GetPositions() = 0;
	virtual void*		GetColors() = 0;
#if USE_HDR
	virtual void*		GetFloatColors1() = 0;
	virtual void*		GetFloatColors2() = 0;
	virtual void*		GetFloatColors3() = 0;
#endif
	virtual void*		GetUVs() = 0;
	virtual void*		GetNormals() = 0;
	virtual void*		GetTangents() = 0;
	virtual void*		GetBoneIndices() = 0;
	virtual void*		GetBoneWeights() = 0;

	virtual uint		GetNumVertices() = 0;
	virtual void		SetNumVertices( uint NumVertices ) = 0;

	virtual int			AddReference() = 0;
	virtual int			Release() = 0;

	virtual void		DeviceRelease() = 0;
	virtual void		DeviceReset() = 0;
	virtual void		RegisterDeviceResetCallback( const SDeviceResetCallback& Callback ) = 0;

	virtual void*		Lock( EVertexElements VertexType ) = 0;
	virtual void		Unlock( EVertexElements VertexType ) = 0;
};

#endif // IVERTEXBUFFER_H