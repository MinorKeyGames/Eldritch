#ifndef LINEBATCHER_H
#define LINEBATCHER_H

// Currently, this is intended only for batching debug lines, and
// will have no effect on Release or Final.

#include "array.h"
#include "iindexbuffer.h"
#include "vector.h"

class IRenderer;
class Mesh;
class View;

class LineBatcher
{
public:
	LineBatcher( IRenderer* pRenderer, uint MaterialFlags, bool IsDebug );
	~LineBatcher();

	void	Add( const Array< Vector >& Positions, const Array< uint >& Colors, const Array< index_t >& Indices );
	Mesh*	Render();

	void	DrawLine( const Vector& Start, const Vector& End, unsigned int Color );
	void	DrawTriangle( const Vector& A, const Vector& B, const Vector& C, unsigned int Color );
	void	DrawBox( const Vector& Min, const Vector& Max, unsigned int Color );
	void	DrawX( const Vector& Center, const float Radius, const unsigned int Color );
	void	DrawFrustum( const View& View, unsigned int Color );
	void	DrawSphere( const Vector& Center, float Radius, unsigned int Color );
	void	DrawEllipsoid( const Vector& Center, const Vector& Extents, unsigned int Color );
	void	DrawGradientLine( const Vector& Start, const Vector& End, unsigned int ColorA, unsigned int ColorB );
	void	DrawIrradianceCross( const Vector& Center, const float Radius, const Vector4 Colors[6] );

private:
	Array< Vector >		m_Positions;
	Array< uint >		m_Colors;
	Array< index_t >	m_Indices;
	uint				m_MaterialFlags;
	IRenderer*			m_Renderer;
	bool				m_IsDebug;

	Array< Vector >		m_AddPositions;
	Array< uint >		m_AddColors;
	Array< index_t >	m_AddIndices;
};

#endif // LINEBATCHER_H