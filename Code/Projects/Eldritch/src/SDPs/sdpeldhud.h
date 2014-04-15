#ifndef SDPELDHUD_H
#define SDPELDHUD_H

#include "SDPs/sdpbase.h"

class SDPEldHUD : public SDPBase
{
public:
	SDPEldHUD();
	virtual ~SDPEldHUD();

	DEFINE_SDP_FACTORY( EldHUD );

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const;
};

#endif // SDPELDHUD_H