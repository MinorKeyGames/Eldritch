#ifndef D3D9SHADERPROGRAM_H
#define D3D9SHADERPROGRAM_H

#include "ishaderprogram.h"

class D3D9ShaderProgram : public IShaderProgram
{
public:
	D3D9ShaderProgram();
	virtual ~D3D9ShaderProgram();

	virtual void			Initialize( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration );
	virtual void*			GetHandle() { return NULL; }
	virtual IVertexShader*	GetVertexShader() const { return m_VertexShader; }
	virtual IPixelShader*	GetPixelShader() const { return m_PixelShader; }
	virtual bool			GetVertexShaderRegister( const HashedString& Parameter, uint& Register ) const;
	virtual bool			GetPixelShaderRegister( const HashedString& Parameter, uint& Register ) const;

protected:
	IVertexShader*	m_VertexShader;
	IPixelShader*	m_PixelShader;
};


#endif // D3D9SHADERPROGRAM_H