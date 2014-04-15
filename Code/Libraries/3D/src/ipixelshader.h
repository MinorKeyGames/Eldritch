#ifndef IPIXELSHADER_H
#define IPIXELSHADER_H

class IPixelShader
{
public:
	virtual ~IPixelShader() {}

	virtual void*	GetHandle() = 0;
	virtual bool	GetRegister( const HashedString& Parameter, uint& Register ) const = 0;
};

#endif // IPIXELSHADER_H