#include "core.h"
#include "sdpbase.h"
#include "irenderer.h"
#include "mesh.h"
#include "view.h"
#include "bonearray.h"
#include "reversehash.h"
#include "simplestring.h"

SDPBase::SDPBase()
{
}

SDPBase::~SDPBase()
{
}

/*virtual*/ void SDPBase::SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const
{
	Unused( View );

	const Matrix WorldMatrix	= pRenderer->GetWorldMatrix();
	const Matrix WVP			= WorldMatrix * pRenderer->GetViewMatrix() * pRenderer->GetProjectionMatrix();

	STATIC_HASHED_STRING( WorldMatrix );
	pRenderer->SetVertexShaderMatrix( sWorldMatrix, WorldMatrix.GetArray(), 1 );

	STATIC_HASHED_STRING( WVP );
	pRenderer->SetVertexShaderMatrix( sWVP, WVP.GetArray(), 1 );

	if( pMesh->IsAnimated() )
	{
		STATIC_HASHED_STRING( BoneMatrices );
		pRenderer->SetVertexShaderMatrix( sBoneMatrices, pMesh->m_BoneMatrices[0].GetArray(), pMesh->m_Bones->GetNumBones() );
	}
}