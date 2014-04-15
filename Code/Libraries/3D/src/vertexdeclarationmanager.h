#ifndef VERTEXDECLARATIONMANAGER_H
#define VERTEXDECLARATIONMANAGER_H

#include "map.h"

class IVertexDeclaration;
class IRenderer;

class VertexDeclarationManager
{
public:
	VertexDeclarationManager( IRenderer* pRenderer );
	~VertexDeclarationManager();

	IVertexDeclaration*	GetVertexDeclaration( uint VertexSignature );
	void				FreeVertexDeclarations();

private:
	Map< uint, IVertexDeclaration* >	m_VertexDeclarations;
	IRenderer*							m_Renderer;
};

#endif