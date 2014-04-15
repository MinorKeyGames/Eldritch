#ifndef GL2TEXTURE_H
#define GL2TEXTURE_H

#include "texturecommon.h"
#include "gl2.h"
#include "renderstates.h"

class GL2Texture : public TextureCommon
{
public:
	GL2Texture();
	GL2Texture( GLuint Texture );
	virtual ~GL2Texture();

	virtual void*	GetHandle();

	SSamplerState&	GetSamplerState() { return m_SamplerState; }

private:
	virtual void	CreateTexture( byte* const ARGBImage );
	virtual void	CreateTextureFromDDS( const IDataStream& Stream );

	GLuint			m_Texture;

	// Shadow sampler state per-texture in GL, because that's how GL manages sampler state.
	SSamplerState	m_SamplerState;
};

#endif // GL2TEXTURE_H
