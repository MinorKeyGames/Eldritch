#include "core.h"
#include "gl2vertexbuffer.h"
#include "vector.h"
#include "vector2.h"
#include "vector4.h"

GL2VertexBuffer::GL2VertexBuffer()
:	m_RefCount( 0 )
,	m_NumVertices( 0 )
,	m_Dynamic( false )
,	m_PositionsVBO( 0 )
,	m_ColorsVBO( 0 )
#if USE_HDR
,	m_FloatColors1VBO( 0 )
,	m_FloatColors2VBO( 0 )
,	m_FloatColors3VBO( 0 )
#endif
,	m_UVsVBO( 0 )
,	m_NormalsVBO( 0 )
,	m_TangentsVBO( 0 )
,	m_BoneIndicesVBO( 0 )
,	m_BoneWeightsVBO( 0 )
{
}

GL2VertexBuffer::~GL2VertexBuffer()
{
	DeviceRelease();
}

int GL2VertexBuffer::AddReference()
{
	++m_RefCount;
	return m_RefCount;
}

int GL2VertexBuffer::Release()
{
	DEVASSERT( m_RefCount > 0 );
	--m_RefCount;
	if( m_RefCount <= 0 )
	{
		delete this;
		return 0;
	}
	return m_RefCount;
}

void GL2VertexBuffer::DeviceRelease()
{
#define SAFEDELETEBUFFER( USE ) if( m_##USE##VBO != 0 ) { glDeleteBuffers( 1, &m_##USE##VBO ); }
	SAFEDELETEBUFFER( Positions );
	SAFEDELETEBUFFER( Colors );
	SAFEDELETEBUFFER( FloatColors1 );
	SAFEDELETEBUFFER( FloatColors2 );
	SAFEDELETEBUFFER( FloatColors3 );
	SAFEDELETEBUFFER( UVs );
	SAFEDELETEBUFFER( Normals );
	SAFEDELETEBUFFER( Tangents );
	SAFEDELETEBUFFER( BoneIndices );
	SAFEDELETEBUFFER( BoneWeights );
#undef SAFEDELETEBUFFER
}

void GL2VertexBuffer::DeviceReset()
{
}

void GL2VertexBuffer::RegisterDeviceResetCallback( const SDeviceResetCallback& Callback )
{
	Unused( Callback );
}

void GL2VertexBuffer::Init( const SInit& InitStruct )
{
	XTRACE_FUNCTION;

	m_NumVertices	= InitStruct.NumVertices;
	m_Dynamic		= InitStruct.Dynamic;

	const GLenum Usage = InitStruct.Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

#define CREATEBUFFER( USE, TYPE )																			\
	if( InitStruct.USE )																					\
	{																										\
		glGenBuffers( 1, &m_##USE##VBO );																	\
		ASSERT( m_##USE##VBO != 0 );																		\
		glBindBuffer( GL_ARRAY_BUFFER, m_##USE##VBO );														\
		glBufferData( GL_ARRAY_BUFFER, InitStruct.NumVertices * sizeof( TYPE ), InitStruct.USE, Usage );	\
	}

	CREATEBUFFER( Positions, Vector );
	CREATEBUFFER( Colors, uint );
#if USE_HDR
	CREATEBUFFER( FloatColors1, Vector4 );
	CREATEBUFFER( FloatColors2, Vector4 );
	CREATEBUFFER( FloatColors3, Vector4 );
#endif
	CREATEBUFFER( UVs, Vector2 );
	CREATEBUFFER( Normals, Vector );
	CREATEBUFFER( Tangents, Vector4 );
	CREATEBUFFER( BoneIndices, SBoneData );
	CREATEBUFFER( BoneWeights, SBoneData );

#undef CREATEBUFFER
}

void* GL2VertexBuffer::GetPositions()
{
	return &m_PositionsVBO;
}

void* GL2VertexBuffer::GetColors()
{
	return &m_ColorsVBO;
}

#if USE_HDR
void* GL2VertexBuffer::GetFloatColors1()
{
	return &m_FloatColors1VBO;
}

void* GL2VertexBuffer::GetFloatColors2()
{
	return &m_FloatColors2VBO;
}

void* GL2VertexBuffer::GetFloatColors3()
{
	return &m_FloatColors3VBO;
}
#endif

void* GL2VertexBuffer::GetUVs()
{
	return &m_UVsVBO;
}

void* GL2VertexBuffer::GetNormals()
{
	return &m_NormalsVBO;
}

void* GL2VertexBuffer::GetTangents()
{
	return &m_TangentsVBO;
}

void* GL2VertexBuffer::GetBoneIndices()
{
	return &m_BoneIndicesVBO;
}

void* GL2VertexBuffer::GetBoneWeights()
{
	return &m_BoneWeightsVBO;
}

uint GL2VertexBuffer::GetNumVertices()
{
	return m_NumVertices;
}

void GL2VertexBuffer::SetNumVertices( uint NumVertices )
{
	m_NumVertices = NumVertices;
}

void* GL2VertexBuffer::Lock( IVertexBuffer::EVertexElements VertexType )
{
	ASSERT( m_Dynamic );

	const GLuint VBO = InternalGetVBO( VertexType );
	ASSERT( VBO != 0 );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );

	void* const pData = glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY );
	ASSERT( pData );

	return pData;
}

void GL2VertexBuffer::Unlock( EVertexElements VertexType )
{
	ASSERT( m_Dynamic );

	const GLuint VBO = InternalGetVBO( VertexType );
	ASSERT( VBO != 0 );

	glBindBuffer( GL_ARRAY_BUFFER, VBO );

	const GLboolean Success = glUnmapBuffer( GL_ARRAY_BUFFER );
	ASSERT( Success == GL_TRUE );
	Unused( Success );
}

GLuint GL2VertexBuffer::InternalGetVBO( EVertexElements VertexType )
{
	switch( VertexType )
	{
	case EVE_Positions:
		return m_PositionsVBO;
	case EVE_Colors:
		return m_ColorsVBO;
#if USE_HDR
	case EVE_FloatColors1:
		return m_FloatColors1VBO;
	case EVE_FloatColors2:
		return m_FloatColors2VBO;
	case EVE_FloatColors3:
		return m_FloatColors3VBO;
#endif
	case EVE_UVs:
		return m_UVsVBO;
	case EVE_Normals:
		return m_NormalsVBO;
	case EVE_Tangents:
		return m_TangentsVBO;
	case EVE_BoneIndices:
		return m_BoneIndicesVBO;
	case EVE_BoneWeights:
		return m_BoneWeightsVBO;
	default:
		WARN;
		return 0;
	}
}