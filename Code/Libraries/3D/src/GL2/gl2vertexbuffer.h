#ifndef GL2VERTEXBUFFER_H
#define GL2VERTEXBUFFER_H

#include "3d.h"
#include "ivertexbuffer.h"
#include "gl2.h"

class GL2VertexBuffer : public IVertexBuffer
{
public:
	GL2VertexBuffer();
	virtual ~GL2VertexBuffer();

	virtual void		Init( const SInit& InitStruct );

	virtual void*		GetPositions();
	virtual void*		GetColors();
#if USE_HDR
	virtual void*		GetFloatColors1();
	virtual void*		GetFloatColors2();
	virtual void*		GetFloatColors3();
#endif
	virtual void*		GetUVs();
	virtual void*		GetNormals();
	virtual void*		GetTangents();
	virtual void*		GetBoneIndices();
	virtual void*		GetBoneWeights();

	virtual uint		GetNumVertices();
	virtual void		SetNumVertices( uint NumVertices );

	virtual int			AddReference();
	virtual int			Release();

	virtual void		DeviceRelease();
	virtual void		DeviceReset();
	virtual void		RegisterDeviceResetCallback( const SDeviceResetCallback& Callback );

	virtual void*		Lock( EVertexElements VertexType );
	virtual void		Unlock( EVertexElements VertexType );

private:
	GLuint				InternalGetVBO( EVertexElements VertexType );

	int		m_RefCount;
	uint	m_NumVertices;
	bool	m_Dynamic;

	GLuint	m_PositionsVBO;
	GLuint	m_ColorsVBO;
#if USE_HDR
	GLuint	m_FloatColors1VBO;
	GLuint	m_FloatColors2VBO;
	GLuint	m_FloatColors3VBO;
#endif
	GLuint	m_UVsVBO;
	GLuint	m_NormalsVBO;
	GLuint	m_TangentsVBO;
	GLuint	m_BoneIndicesVBO;
	GLuint	m_BoneWeightsVBO;
};

#endif // GL2VERTEXBUFFER_H
