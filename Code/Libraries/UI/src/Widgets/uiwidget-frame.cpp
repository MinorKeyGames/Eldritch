#include "core.h"
#include "uiwidget-frame.h"
#include "ivertexbuffer.h"
#include "iindexbuffer.h"
#include "ivertexdeclaration.h"
#include "uimanager.h"
#include "mesh.h"
#include "meshfactory.h"
#include "shadermanager.h"
#include "configmanager.h"
#include "irenderer.h"
#include "mathcore.h"
#include "texturemanager.h"

UIWidgetFrame::UIWidgetFrame()
:	m_Texture( NULL )
,	m_Dimensions()
,	m_Border( 0.0f )
,	m_Mesh( NULL )
,	m_Material()
{
}

UIWidgetFrame::~UIWidgetFrame()
{
	SafeDelete( m_Mesh );
}

Mesh* UIWidgetFrame::CreateMesh() const
{
	static const uint kNumVertices	= 16;
	static const uint kNumIndices	= 54;

	const Vector TopLeft		= Vector( m_TopLeft.x, 0.0f, m_TopLeft.y );
	const Vector BottomRight	= TopLeft + Vector( m_Dimensions.x, 0.0f, m_Dimensions.y );

	Array<Vector> Positions;
	Positions.Reserve( kNumVertices );

	{
		Array<float> PositionsX;
		PositionsX.Reserve( 4 );

		PositionsX.PushBack( TopLeft.x );
		PositionsX.PushBack( TopLeft.x + m_Border );
		PositionsX.PushBack( BottomRight.x - m_Border );
		PositionsX.PushBack( BottomRight.x );

		Array<float> PositionsY;
		PositionsY.Reserve( 4 );

		PositionsY.PushBack( TopLeft.z );
		PositionsY.PushBack( TopLeft.z + m_Border );
		PositionsY.PushBack( BottomRight.z - m_Border );
		PositionsY.PushBack( BottomRight.z );

		for( uint Y = 0; Y < 4; ++Y )
		{
			for( uint X = 0; X < 4; ++X )
			{
				Positions.PushBack( Vector( PositionsX[ X ], 0.0f, PositionsY[ Y ] ) );
			}
		}
	}

	Array<Vector2> UVs;
	UVs.Reserve( kNumVertices );

	{
		Array<float> UVValues;
		UVValues.Reserve( 4 );

		UVValues.PushBack( 0.0f );
		UVValues.PushBack( 0.25f );
		UVValues.PushBack( 0.75f );
		UVValues.PushBack( 1.0f );

		for( uint V = 0; V < 4; ++V )
		{
			for( uint U = 0; U < 4; ++U )
			{
				UVs.PushBack( Vector2( UVValues[ U ], UVValues[ V ] ) );
			}
		}
	}

	Array<index_t> Indices;
	Indices.Reserve( kNumIndices );

	for( index_t Y = 0; Y < 3; ++Y )
	{
		for( index_t X = 0; X < 3; ++X )
		{
			const index_t Base = X + Y * 4;
			Indices.PushBack( Base + 0 );
			Indices.PushBack( Base + 4 );
			Indices.PushBack( Base + 1 );
			Indices.PushBack( Base + 4 );
			Indices.PushBack( Base + 5 );
			Indices.PushBack( Base + 1 );
		}
	}

	IRenderer* const			pRenderer			= m_UIManager->GetRenderer();
	IVertexBuffer* const		VertexBuffer		= pRenderer->CreateVertexBuffer();
	IVertexDeclaration* const	VertexDeclaration	= pRenderer->GetVertexDeclaration( VD_POSITIONS | VD_UVS );
	IIndexBuffer* const			IndexBuffer			= pRenderer->CreateIndexBuffer();

	IVertexBuffer::SInit InitStruct;
	InitStruct.NumVertices	= kNumVertices;
	InitStruct.Positions	= Positions.GetData();
	InitStruct.UVs			= UVs.GetData();

	VertexBuffer->Init( InitStruct );
	IndexBuffer->Init( kNumIndices, Indices.GetData() );
	IndexBuffer->SetPrimitiveType( EPT_TRIANGLELIST );

	Mesh* const pMesh = new Mesh( VertexBuffer, VertexDeclaration, IndexBuffer );

	pMesh->m_AABB = AABB( TopLeft, BottomRight );

	return pMesh;
}

void UIWidgetFrame::UpdateRender()
{
	DEBUGASSERT( m_UIManager->DEBUGIsSafeToUpdateRender( m_LastRenderedTime ) );

	SafeDelete( m_Mesh );

	IRenderer* const pRenderer = m_UIManager->GetRenderer();
	m_Mesh = CreateMesh();
	m_Mesh->SetMaterialDefinition( m_Material, pRenderer );
	m_Mesh->SetTexture( 0, m_Texture );
	m_Mesh->SetMaterialFlags( m_RenderInWorld ? MAT_INWORLDHUD : MAT_HUD );
}

void UIWidgetFrame::Render( bool HasFocus )
{
	XTRACE_FUNCTION;

	UIWidget::Render( HasFocus );

	if( IsDisabled() )
	{
		m_Mesh->m_ConstantColor = m_DisabledColor;
	}
	else if( HasFocus )
	{
		m_Mesh->m_ConstantColor = GetHighlightColor();
	}
	else
	{
		m_Mesh->m_ConstantColor = m_Color;
	}

	// Don't draw mesh if it's going to be invisible
	if( m_Mesh->m_ConstantColor.a > 0.0f )
	{
		m_UIManager->GetRenderer()->AddMesh( m_Mesh );
	}
}

void UIWidgetFrame::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	UIWidget::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );
	MAKEHASH( m_Archetype );

	STATICHASH( DisplayWidth );
	const float DisplayWidth	= ConfigManager::GetFloat( sDisplayWidth );
	const float ParentWidth		= m_OriginParent ? m_OriginParent->GetWidth() : DisplayWidth;
	const float ParentX			= m_OriginParent ? Ceiling( m_OriginParent->GetX() ) : 0.0f;

	STATICHASH( DisplayHeight );
	const float DisplayHeight	= ConfigManager::GetFloat( sDisplayHeight );
	const float ParentHeight	= m_OriginParent ? m_OriginParent->GetHeight() : DisplayHeight;
	const float ParentY			= m_OriginParent ? Ceiling( m_OriginParent->GetY() ) : 0.0f;

	// "Screen" values are now relative to parent, which may or may not be screen dimensions.

	STATICHASH( PixelX );
	STATICHASH( ScreenX );
	float X = Pick(
		ConfigManager::GetArchetypeFloat( sPixelX, sm_Archetype, 0.0f, sDefinitionName ),
		ParentWidth * ConfigManager::GetArchetypeFloat( sScreenX, sm_Archetype, 0.0f, sDefinitionName ) );

	STATICHASH( PixelY );
	STATICHASH( ScreenY );
	float Y = Pick(
		ConfigManager::GetArchetypeFloat( sPixelY, sm_Archetype, 0.0f, sDefinitionName ),
		ParentHeight * ConfigManager::GetArchetypeFloat( sScreenY, sm_Archetype, 0.0f, sDefinitionName ) );
	
	STATICHASH( PixelWidth );
	STATICHASH( ScreenWidth );
	float Width = Pick(
		ConfigManager::GetArchetypeFloat( sPixelWidth, sm_Archetype, 0.0f, sDefinitionName ),
		ParentWidth * ConfigManager::GetArchetypeFloat( sScreenWidth, sm_Archetype, 0.0f, sDefinitionName ) );
	
	STATICHASH( PixelHeight );
	STATICHASH( ScreenHeight );
	float Height = Pick(
		ConfigManager::GetArchetypeFloat( sPixelHeight, sm_Archetype, 0.0f, sDefinitionName ),
		ParentHeight * ConfigManager::GetArchetypeFloat( sScreenHeight, sm_Archetype, 0.0f, sDefinitionName ) );

	STATICHASH( PixelBorder );
	m_Border = ConfigManager::GetArchetypeFloat( sPixelBorder, sm_Archetype, 0.0f, sDefinitionName );

	// Adjust for desired aspect ratio if one dimension is not given
	// (This is used to size images using ScreenWidth or ScreenHeight
	// properly regardless of screen aspect ratio.
	STATICHASH( AspectRatio );
	const float AspectRatio = ConfigManager::GetArchetypeFloat( sAspectRatio, sm_Archetype, 1.0f, sDefinitionName );
	if( Width == 0.0f )
	{
		Width = Height * AspectRatio;
	}
	else if( Height == 0.0f )
	{
		Height = Width / AspectRatio;
	}

	AdjustDimensionsToParent( X, Y, Width, Height, ParentX, ParentY, ParentWidth, ParentHeight );
	GetPositionFromOrigin( X, Y, Width, Height );

	ASSERT( Width > m_Border * 2.0f );
	ASSERT( Height > m_Border * 2.0f );

	STATICHASH( ClampToPixelGrid );
	if( ConfigManager::GetArchetypeBool( sClampToPixelGrid, sm_Archetype, true, sDefinitionName ) )
	{
		m_TopLeft.x = Round( m_TopLeft.x );
		m_TopLeft.y = Round( m_TopLeft.y );
	}

	// Offset to properly align on pixel grid.
	const float PixelGridOffset = GetPixelGridOffset();
	m_TopLeft.x -= PixelGridOffset;
	m_TopLeft.y -= PixelGridOffset;

	STATICHASH( Image );
	const char* const Filename = ConfigManager::GetArchetypeString( sImage, sm_Archetype, DEFAULT_TEXTURE, sDefinitionName );
	m_Texture = m_UIManager->GetRenderer()->GetTextureManager()->GetTexture( Filename, TextureManager::ETL_Permanent );

	m_Dimensions = Vector2( Width, Height );

	STATICHASH( MaterialOverride );
	const SimpleString DefaultMaterial( "Material_HUD" );
	m_Material = ConfigManager::GetArchetypeString( sMaterialOverride, sm_Archetype, DefaultMaterial.CStr(), sDefinitionName );

	UpdateRender();
}

void UIWidgetFrame::GetBounds( SRect& OutBounds )
{
	OutBounds = SRect( m_TopLeft.x, m_TopLeft.y, m_TopLeft.x + m_Dimensions.x, m_TopLeft.y + m_Dimensions.y );
}