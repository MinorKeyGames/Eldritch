#include "core.h"
#include "3d.h"
#include "uiwidget-image.h"
#include "uimanager.h"
#include "mesh.h"
#include "meshfactory.h"
#include "shadermanager.h"
#include "configmanager.h"
#include "irenderer.h"
#include "mathcore.h"
#include "texturemanager.h"

UIWidgetImage::UIWidgetImage()
:	m_Texture( NULL )
,	m_Dimensions()
,	m_Mesh( NULL )
,	m_Calibration( false )
,	m_Material()
{
}

UIWidgetImage::UIWidgetImage( const SimpleString& DefinitionName )
:	m_Texture( NULL )
,	m_Dimensions()
,	m_Mesh( NULL )
,	m_Calibration( false )
,	m_Material()
{
	InitializeFromDefinition( DefinitionName );
}

UIWidgetImage::~UIWidgetImage()
{
	SafeDelete( m_Mesh );
}

void UIWidgetImage::UpdateRender()
{
	// This will fail if UpdateRender is called between submitting UI meshes to the renderer and actually
	// rendering the scene. It may actually be safe if this widget was not submitted to the renderer.
	DEBUGASSERT( m_UIManager->DEBUGIsSafeToUpdateRender( m_LastRenderedTime ) );

	SafeDelete( m_Mesh );

	IRenderer* pRenderer = m_UIManager->GetRenderer();
	m_Mesh = pRenderer->GetMeshFactory()->CreateSprite();
	m_Mesh->SetMaterialDefinition( m_Material, pRenderer );
	m_Mesh->SetTexture( 0, m_Texture );
	m_Mesh->SetMaterialFlags( m_RenderInWorld ? MAT_INWORLDHUD : MAT_HUD );
	m_Mesh->m_Location = Vector( m_TopLeft.x + m_Dimensions.x * 0.5f, 0.0f, m_TopLeft.y + m_Dimensions.y * 0.5f );
	m_Mesh->m_Scale = Vector( m_Dimensions.x, 1.0f, m_Dimensions.y );
}

void UIWidgetImage::Render( bool HasFocus )
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

void UIWidgetImage::InitializeFromDefinition( const SimpleString& DefinitionName )
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

	// Offset relative to resolved image dimensions if specified
	STATICHASH( ImageX );
	X = Pick( X, ConfigManager::GetArchetypeFloat( sImageX, sm_Archetype, 0.0f, sDefinitionName ) * Width );

	STATICHASH( ImageY );
	Y = Pick( Y, ConfigManager::GetArchetypeFloat( sImageY, sm_Archetype, 0.0f, sDefinitionName ) * Height );

	AdjustDimensionsToParent( X, Y, Width, Height, ParentX, ParentY, ParentWidth, ParentHeight );
	GetPositionFromOrigin( X, Y, Width, Height );

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

	// If LoadImage is false, we're expecting to dynamically set the texture in code somewhere
	STATICHASH( LoadImage );
	if( ConfigManager::GetArchetypeBool( sLoadImage, sm_Archetype, true, sDefinitionName ) )
	{
		STATICHASH( Image );
		SetTexture( ConfigManager::GetArchetypeString( sImage, sm_Archetype, DEFAULT_TEXTURE, sDefinitionName ) );
	}

	m_Dimensions = Vector2( Width, Height );

	STATICHASH( Calibration );
	m_Calibration = ConfigManager::GetArchetypeBool( sCalibration, sm_Archetype, false, sDefinitionName );

	STATICHASH( MaterialOverride );
	const SimpleString DefaultMaterial( "Material_HUD" );
	m_Material = ConfigManager::GetArchetypeString( sMaterialOverride, sm_Archetype, DefaultMaterial.CStr(), sDefinitionName );

	UpdateRender();
}

void UIWidgetImage::GetBounds( SRect& OutBounds )
{
	OutBounds = SRect( m_TopLeft.x, m_TopLeft.y, m_TopLeft.x + m_Dimensions.x, m_TopLeft.y + m_Dimensions.y );
}

void UIWidgetImage::SetTexture( const char* Filename )
{
	ASSERT( Filename );
	m_Texture = m_UIManager->GetRenderer()->GetTextureManager()->GetTexture( Filename, TextureManager::ETL_Permanent );
}

// HACK: Supporting Eldritch character screen.
// Note that we call UpdateRender here since image widgets don't normally refresh their textures.
void UIWidgetImage::SetTexture( ITexture* const pTexture )
{
	m_Texture = pTexture;
	UpdateRender();
}