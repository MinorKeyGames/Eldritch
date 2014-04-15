#ifndef GL2INDEXBUFFER_H
#define GL2INDEXBUFFER_H

#include "iindexbuffer.h"
#include "gl2.h"

#if USE_LONG_INDICES
#define GLINDEXFORMAT GL_UNSIGNED_INT
#else
#define GLINDEXFORMAT GL_UNSIGNED_SHORT
#endif

class GL2IndexBuffer : public IIndexBuffer
{
public:
	GL2IndexBuffer();
	virtual ~GL2IndexBuffer();

	virtual void	Init( uint NumIndices, index_t* Indices );

	virtual void*	GetIndices();

	virtual uint	GetNumIndices();
	virtual void	SetNumIndices( uint NumIndices );
	virtual uint	GetNumPrimitives();

	virtual void	SetPrimitiveType( EPrimitiveType PrimitiveType );

	virtual int		AddReference();
	virtual int		Release();

	GLenum			GetPrimitiveType() { return m_PrimitiveType; }

private:
	int			m_RefCount;
	uint		m_NumIndices;
	GLenum		m_PrimitiveType;

	GLuint		m_IndicesVBO;
};

#endif // GL2INDEXBUFFER_H