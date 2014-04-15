#include "core.h"
#include "gl2indexbuffer.h"

// Maps my EPrimitiveType to GL's primitive enumerations
GLenum PrimitiveTypeTable[] =
{
	GL_POINTS,
	GL_LINES,
	GL_LINE_STRIP,
	GL_TRIANGLES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
};

GL2IndexBuffer::GL2IndexBuffer()
:	m_RefCount( 0 )
,	m_NumIndices( 0 )
,	m_PrimitiveType( GL_TRIANGLES )
,	m_IndicesVBO( 0 )
{
}

GL2IndexBuffer::~GL2IndexBuffer()
{
	if( m_IndicesVBO )
	{
		glDeleteBuffers( 1, &m_IndicesVBO );
		m_IndicesVBO = 0;
	}
}

int GL2IndexBuffer::AddReference()
{
	++m_RefCount;
	return m_RefCount;
}

int GL2IndexBuffer::Release()
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

void GL2IndexBuffer::Init( uint NumIndices, index_t* Indices )
{
	XTRACE_FUNCTION;

	m_NumIndices = NumIndices;

	if( Indices )
	{
		glGenBuffers( 1, &m_IndicesVBO );
		ASSERT( m_IndicesVBO != 0 );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_IndicesVBO );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, NumIndices * sizeof( index_t ), Indices, GL_STATIC_DRAW );
	}
}

void* GL2IndexBuffer::GetIndices()
{
	return &m_IndicesVBO;
}

uint GL2IndexBuffer::GetNumIndices()
{
	return m_NumIndices;
}

void GL2IndexBuffer::SetNumIndices( uint NumIndices )
{
	m_NumIndices = NumIndices;
}

uint GL2IndexBuffer::GetNumPrimitives()
{
	switch( m_PrimitiveType )
	{
	case GL_POINTS:
		return m_NumIndices;
	case GL_LINES:
		return m_NumIndices >> 1;
	case GL_LINE_STRIP:
		return m_NumIndices - 1;
	case GL_TRIANGLES:
		return m_NumIndices / 3;
	case GL_TRIANGLE_STRIP:
		return m_NumIndices - 2;
	case GL_TRIANGLE_FAN:
		return m_NumIndices - 2;
	default:
		return 0;
	}
}

void GL2IndexBuffer::SetPrimitiveType( EPrimitiveType PrimitiveType )
{
	m_PrimitiveType = PrimitiveTypeTable[ PrimitiveType ];
}