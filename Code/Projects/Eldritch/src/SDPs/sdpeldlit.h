#ifndef SDPELDLIT_H
#define SDPELDLIT_H

#include "SDPs/sdpbase.h"

struct SVoxelIrradiance;

class SDPEldLit : public SDPBase
{
public:
	SDPEldLit();
	virtual ~SDPEldLit();

	DEFINE_SDP_FACTORY( EldLit );

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const;
};

#endif // SDPELDLIT_H