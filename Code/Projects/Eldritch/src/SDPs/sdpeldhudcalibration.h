#ifndef SDPELDHUDCALIBRATION_H
#define SDPELDHUDCALIBRATION_H

#include "SDPs/sdpbase.h"

class SDPEldHUDCalibration : public SDPBase
{
public:
	SDPEldHUDCalibration();
	virtual ~SDPEldHUDCalibration();

	DEFINE_SDP_FACTORY( EldHUDCalibration );

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const;
};

#endif // SDPELDHUDCALIBRATION_H