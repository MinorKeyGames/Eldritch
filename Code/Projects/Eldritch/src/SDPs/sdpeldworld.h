#ifndef SDPELDWORLD_H
#define SDPELDWORLD_H

#include "SDPs/sdpbase.h"

class SDPEldWorld : public SDPBase
{
public:
	SDPEldWorld();
	virtual ~SDPEldWorld();

	DEFINE_SDP_FACTORY( EldWorld );

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const;
};

#endif // SDPELDWORLD_H