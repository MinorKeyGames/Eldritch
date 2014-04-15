#include "core.h"
#include "sdpeldlit.h"
#include "irenderer.h"
#include "mesh.h"
#include "view.h"
#include "eldritchframework.h"
#include "eldritchgame.h"
#include "eldritchmesh.h"
#include "vector4.h"

SDPEldLit::SDPEldLit()
{
}

SDPEldLit::~SDPEldLit()
{
}

/*virtual*/ void SDPEldLit::SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const
{
	SDPBase::SetShaderParameters( pRenderer, pMesh, View );

	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	EldritchGame* const			pGame			= pFramework->GetGame();
	const Vector4				FogParams		= pGame->GetFogParams();
	const Vector4				FogPosition		= View.m_Location;

	STATIC_HASHED_STRING( FogParams );
	pRenderer->SetPixelShaderFloat4( sFogParams,		FogParams.GetArray(),	1 );

	STATIC_HASHED_STRING( FogViewPosition );
	pRenderer->SetPixelShaderFloat4( sFogViewPosition,	FogPosition.GetArray(),	1 );

	EldritchMesh* const pEldritchMesh = static_cast<EldritchMesh*>( pMesh );
	STATIC_HASHED_STRING( LightCube );
	const SVoxelIrradiance& Irradiance = pEldritchMesh->GetIrradianceCube();
	pRenderer->SetPixelShaderFloat4( sLightCube, Irradiance.m_Light[0].GetArray(), 6 );
}