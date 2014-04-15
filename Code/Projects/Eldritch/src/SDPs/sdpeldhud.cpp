#include "core.h"
#include "sdpeldhud.h"
#include "irenderer.h"
#include "mesh.h"
#include "view.h"

SDPEldHUD::SDPEldHUD()
{
}

SDPEldHUD::~SDPEldHUD()
{
}

/*virtual*/ void SDPEldHUD::SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const
{
	SDPBase::SetShaderParameters( pRenderer, pMesh, View );

	STATIC_HASHED_STRING( ConstantColor );
	pRenderer->SetPixelShaderFloat4( sConstantColor, pMesh->m_ConstantColor.GetArray(), 1 );
}