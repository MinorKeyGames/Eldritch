#ifndef SDPELDDROPSHADOW_H
#define SDPELDDROPSHADOW_H

#include "SDPs/sdpbase.h"

class SDPEldDropShadow : public SDPBase
{
public:
	SDPEldDropShadow();
	virtual ~SDPEldDropShadow();

	DEFINE_SDP_FACTORY( EldDropShadow );

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const;
};

#endif // SDPELDDROPSHADOW_H