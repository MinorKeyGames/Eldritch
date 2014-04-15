#include "core.h"
#include "gl2vertexdeclaration.h"

GL2VertexDeclaration::GL2VertexDeclaration()
:	m_VertexSignature( 0 )
{
}

GL2VertexDeclaration::~GL2VertexDeclaration()
{
}

void GL2VertexDeclaration::Initialize( uint VertexSignature )
{
	m_VertexSignature = VertexSignature;
}

void* GL2VertexDeclaration::GetDeclaration()
{
	return NULL;
}

uint GL2VertexDeclaration::GetSignature()
{
	return m_VertexSignature;
}