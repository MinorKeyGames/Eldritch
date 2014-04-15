#include "core.h"
#include "sdpeldhudcalibration.h"
#include "irenderer.h"
#include "mesh.h"
#include "view.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "vector4.h"

SDPEldHUDCalibration::SDPEldHUDCalibration()
{
}

SDPEldHUDCalibration::~SDPEldHUDCalibration()
{
}

/*virtual*/ void SDPEldHUDCalibration::SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const
{
	SDPBase::SetShaderParameters( pRenderer, pMesh, View );

	EldritchFramework* const	pFramework	= EldritchFramework::GetInstance();
	EldritchGame* const			pGame		= pFramework->GetGame();
	const Vector4				Gamma		= Vector4( pGame->GetGamma(), 0.0f, 0.0f, 0.0f );

	STATIC_HASHED_STRING( Gamma );
	pRenderer->SetPixelShaderFloat4( sGamma, Gamma.GetArray(), 1 );

	STATIC_HASHED_STRING( ConstantColor );
	pRenderer->SetPixelShaderFloat4( sConstantColor, pMesh->m_ConstantColor.GetArray(), 1 );
}