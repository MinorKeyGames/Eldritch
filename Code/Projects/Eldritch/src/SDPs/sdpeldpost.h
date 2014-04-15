#ifndef SDPELDPOST_H
#define SDPELDPOST_H

#include "SDPs/sdpbase.h"

class SDPEldPost : public SDPBase
{
public:
	SDPEldPost();
	virtual ~SDPEldPost();

	DEFINE_SDP_FACTORY( EldPost );

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const;
};

#endif // SDPELDPOST_H