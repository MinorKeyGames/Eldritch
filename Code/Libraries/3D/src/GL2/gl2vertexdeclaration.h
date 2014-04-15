#ifndef GL2VERTEXDECLARATION_H
#define GL2VERTEXDECLARATION_H

// NOTE: For a more modern GL implementation, I believe a vertex
// array object (VAO) would go here. But I don't need it for now.

#include "ivertexdeclaration.h"

class GL2VertexDeclaration : public IVertexDeclaration
{
public:
	GL2VertexDeclaration();
	virtual ~GL2VertexDeclaration();

	virtual void	Initialize( uint VertexSignature );
	virtual void*	GetDeclaration();
	virtual uint	GetSignature();

private:
	uint	m_VertexSignature;
};

#endif // GL2VERTEXDECLARATION_H
