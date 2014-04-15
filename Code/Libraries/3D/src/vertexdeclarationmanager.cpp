#include "core.h"
#include "3d.h"
#include "vertexdeclarationmanager.h"
#include "irenderer.h"
#include "ivertexdeclaration.h"

VertexDeclarationManager::VertexDeclarationManager( IRenderer* pRenderer )
:	m_VertexDeclarations()
,	m_Renderer( pRenderer ) {}

VertexDeclarationManager::~VertexDeclarationManager()
{
	FreeVertexDeclarations();
}

IVertexDeclaration* VertexDeclarationManager::GetVertexDeclaration( uint VertexSignature )
{
	Map< uint, IVertexDeclaration* >::Iterator VDeclIter = m_VertexDeclarations.Search( VertexSignature );
	if( VDeclIter.IsNull() )
	{
		STATIC_HASHED_STRING( Render );
		CATPRINTF( sRender, 1, "Creating vertex declaration for signature %d\n", VertexSignature );
		IVertexDeclaration* pNewVDecl = m_Renderer->CreateVertexDeclaration();
		pNewVDecl->Initialize( VertexSignature );
		m_VertexDeclarations[ VertexSignature ] = pNewVDecl;
		return pNewVDecl;
	}
	else
	{
		return *VDeclIter;
	}
}

void VertexDeclarationManager::FreeVertexDeclarations()
{
	FOR_EACH_MAP( VDeclIter, m_VertexDeclarations, uint, IVertexDeclaration* )
	{
		SafeDelete( *VDeclIter );
	}
	m_VertexDeclarations.Clear();
}