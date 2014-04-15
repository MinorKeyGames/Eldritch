#ifndef GL2VERTEXSHADER_H
#define GL2VERTEXSHADER_H

#include "ivertexshader.h"
#include "gl2.h"

class IDataStream;

class GL2VertexShader : public IVertexShader
{
public:
	GL2VertexShader();
	virtual ~GL2VertexShader();

	virtual void*	GetHandle() { return &m_VertexShader; }
	virtual bool	GetRegister( const HashedString& Parameter, uint& Register ) const;

	void			Initialize( const IDataStream& Stream );

private:
	GLuint	m_VertexShader;
};

#endif // GL2VERTEXSHADER_H