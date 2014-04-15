#ifndef IVERTEXDECLARATION_H
#define IVERTEXDECLARATION_H

#include "3d.h"

#define VD_POSITIONS		0x0001
#define VD_COLORS			0x0002
#define VD_FLOATCOLORS		0x0004
#define VD_UVS				0x0008
#define VD_NORMALS			0x0010
#define VD_TANGENTS			0x0020
#define VD_BONEINDICES		0x0040
#define VD_BONEWEIGHTS		0x0080
#define VD_BASISCOLORS		0x0304	// == 0x0004 | 0x0100 | 0x0200	(needs three bits for three streams)
#define VD_FLOATCOLORS_SM2	0x0400
#define VD_BASISCOLORS_SM2	0x1C00	// == 0x0400 | 0x0800 | 0x1000	(needs three bits for three streams)

class IVertexDeclaration
{
public:
	virtual ~IVertexDeclaration() {}

	virtual void	Initialize( uint VertexSignature ) = 0;
	virtual void*	GetDeclaration() = 0;
	virtual uint	GetSignature() = 0;
};

#endif // IVERTEXDECLARATION_H