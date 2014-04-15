#include "core.h"
#include "3d.h"
#include "bucket.h"
#include "mesh.h"
#include "view.h"
#include "ishaderprogram.h"

Bucket::Bucket(
	View* pView,
	IRenderTarget* RenderTarget,
	unsigned int Flags,
	unsigned int FilterFlags /*= MAT_NONE*/,
	bool ExclusiveMeshes /*= false*/,
	unsigned int ClearFlags /*= CLEAR_NONE*/,
	Color ClearColor /*= 0xff000000*/,
	float ClearDepth /*= 1.0f*/ )
:	m_View( pView )
,	m_RenderTarget( RenderTarget )
,	m_Flags( Flags )
,	m_FilterFlags( FilterFlags )
,	m_ExclusiveMeshes( ExclusiveMeshes )
,	m_SortByMaterial( false )
,	m_Meshes()
,	m_ClearFlags( ClearFlags )
,	m_ClearColor( ClearColor )
,	m_ClearDepth( ClearDepth )
,	m_SortHelpers()
,	m_MatSortHelpers()
#if BUILD_DEBUG
,	m_DEBUGUseFrustum( false )
,	m_DEBUGFrustumView()
#endif
{
	// Don't reallocate mesh buckets every frame
	m_Meshes.SetDeflate( false );
	m_SortHelpers.SetDeflate( false );
	m_MatSortHelpers.SetDeflate( false );
}

void Bucket::Sort( const View& View )
{
	XTRACE_FUNCTION;

	const Vector&	ViewLocation	= View.m_Location;
	const uint		NumMeshes		= m_Meshes.Size();

	m_SortHelpers.Resize( NumMeshes );

	for( uint MeshIndex = 0; MeshIndex < NumMeshes; ++MeshIndex )
	{
		Mesh* const		pMesh	= m_Meshes[ MeshIndex ];
		DEVASSERT( pMesh );
		SSortHelper&	Helper	= m_SortHelpers[ MeshIndex ];

		// NOTE: Sorting based on distance of offset projected onto view direction
		// would be more correct; but it will rarely make a difference, and is more ops.

		Helper.m_Mesh			= pMesh;
		Helper.m_SortDistanceSq	= ( pMesh->GetSortLocation() - ViewLocation ).LengthSquared();
	}

	m_SortHelpers.InsertionSort();

	for( uint MeshIndex = 0; MeshIndex < NumMeshes; ++MeshIndex )
	{
		const SSortHelper& Helper	= m_SortHelpers[ MeshIndex ];
		m_Meshes[ MeshIndex ]		= Helper.m_Mesh;
	}
}

void Bucket::SortByMaterials()
{
	XTRACE_FUNCTION;

	const uint		NumMeshes		= m_Meshes.Size();
	m_MatSortHelpers.Resize( NumMeshes );

	for( uint MeshIndex = 0; MeshIndex < NumMeshes; ++MeshIndex )
	{
		Mesh* const		pMesh	= m_Meshes[ MeshIndex ];
		DEVASSERT( pMesh );
		SMatSortHelper&	Helper	= m_MatSortHelpers[ MeshIndex ];

		const Material&			Material		= pMesh->GetMaterial();
		IShaderProgram* const	pShaderProgram	= Material.GetShaderProgram();
		DEBUGASSERT( pShaderProgram );

		Helper.m_Mesh			= pMesh;
		Helper.m_VertexShader	= pShaderProgram->GetVertexShader();
		Helper.m_PixelShader	= pShaderProgram->GetPixelShader();
		Helper.m_BaseTexture	= Material.GetTexture( 0 );
	}

	m_MatSortHelpers.InsertionSort();

	for( uint MeshIndex = 0; MeshIndex < NumMeshes; ++MeshIndex )
	{
		const SMatSortHelper& Helper	= m_MatSortHelpers[ MeshIndex ];
		m_Meshes[ MeshIndex ]			= Helper.m_Mesh;
	}
}