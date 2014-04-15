#ifndef ISHADERPROGRAM_H
#define ISHADERPROGRAM_H

class IVertexShader;
class IPixelShader;
class IVertexDeclaration;

class IShaderProgram
{
public:
	virtual ~IShaderProgram() {}

	virtual void			Initialize( IVertexShader* const pVertexShader, IPixelShader* const pPixelShader, IVertexDeclaration* const pVertexDeclaration ) = 0;
	virtual void*			GetHandle() = 0;
	virtual IVertexShader*	GetVertexShader() const = 0;
	virtual IPixelShader*	GetPixelShader() const = 0;
	virtual bool			GetVertexShaderRegister( const HashedString& Parameter, uint& Register ) const = 0;
	virtual bool			GetPixelShaderRegister( const HashedString& Parameter, uint& Register ) const = 0;
};


#endif // ISHADERPROGRAM_H