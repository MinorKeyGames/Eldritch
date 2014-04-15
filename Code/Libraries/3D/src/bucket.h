#ifndef BUCKET_H
#define BUCKET_H

#include "array.h"

#if BUILD_DEBUG
#include "view.h"
#endif

class View;
class IRenderTarget;
class ITexture;
class IVertexShader;
class IPixelShader;
class Mesh;
class View;

class Bucket
{
public:
	Bucket(
		View* pView,
		IRenderTarget* RenderTarget,
		unsigned int Flags,
		unsigned int FilterFlags = 0,
		bool ExclusiveMeshes = false,
		unsigned int ClearFlags = CLEAR_NONE,
		Color ClearColor = 0xff000000,
		float ClearDepth = 1.0f );

	void			Sort( const View& View );	// Sort meshes back-to-front for alpha rendering
	void			SortByMaterials();

	View*			m_View;				// If NULL, the current one will be reused
	IRenderTarget*	m_RenderTarget;		// If NULL, the current one will be reused
	unsigned int	m_Flags;			// MAT_ flags, defined in material.h: a mesh must have all the flags of the bucket to be added
	unsigned int	m_FilterFlags;		// Reject the mesh if it matches any of these flags
	bool			m_ExclusiveMeshes;	// If true, meshes in this bucket are not added to any later buckets
	bool			m_SortByMaterial;	// If true, bucket is sorted by shader and by texture 0
	Array< Mesh* >	m_Meshes;
	uint			m_ClearFlags;
	Color			m_ClearColor;
	float			m_ClearDepth;

	struct SSortHelper
	{
		Mesh*	m_Mesh;
		float	m_SortDistanceSq;

		bool operator<( const SSortHelper& Helper ) const { return m_SortDistanceSq < Helper.m_SortDistanceSq; }
	};

	struct SMatSortHelper
	{
		Mesh*			m_Mesh;
		IVertexShader*	m_VertexShader;
		IPixelShader*	m_PixelShader;
		ITexture*		m_BaseTexture;

		bool operator<( const SMatSortHelper& Helper ) const
		{
			if( m_VertexShader < Helper.m_VertexShader )
			{
				return true;
			}

			if( m_VertexShader > Helper.m_VertexShader )
			{
				return false;
			}

			if( m_PixelShader < Helper.m_PixelShader )
			{
				return true;
			}

			if( m_PixelShader > Helper.m_PixelShader )
			{
				return false;
			}

			if( m_BaseTexture < Helper.m_BaseTexture )
			{
				return true;
			}

			return false;
		}
	};

	Array<SSortHelper>		m_SortHelpers;
	Array<SMatSortHelper>	m_MatSortHelpers;

#if BUILD_DEBUG
	bool			m_DEBUGUseFrustum;	// If true, applies m_DEBUGFrustumView's frustum instead of m_View's
	View			m_DEBUGFrustumView;
#endif
};

#endif // BUCKET_H