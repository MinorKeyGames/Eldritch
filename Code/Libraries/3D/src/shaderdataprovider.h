#ifndef SHADERDATAPROVIDER_H
#define SHADERDATAPROVIDER_H

// Shader data providers are classes which set shader constants/uniforms.
// They are analogous to my old D3D9Shader* class tree.

#define DEFINE_SDP_FACTORY( type ) static class ShaderDataProvider* Factory() { return new SDP##type; }
typedef class ShaderDataProvider* ( *SDPFactoryFunc )( void );

class IRenderer;
class Mesh;
class View;

class ShaderDataProvider
{
public:
	virtual ~ShaderDataProvider();

	virtual void	SetShaderParameters( IRenderer* const pRenderer, Mesh* const pMesh, const View& View ) const = 0;
};

#endif // SHADERDATAPROVIDER_H