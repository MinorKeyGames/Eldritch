#include "core.h"
#include "3d.h"
#include "uiwidget-text.h"
#include "uimanager.h"
#include "font.h"
#include "mesh.h"
#include "shadermanager.h"
#include "configmanager.h"
#include "stringmanager.h"
#include "irenderer.h"
#include "mathcore.h"
#include "fontmanager.h"

UIWidgetText::UIWidgetText()
:	m_Font( NULL )
,	m_IsLiteral( false )
,	m_IsDynamicPosition( false )
,	m_ClampToPixelGrid( false )
,	m_String()
,	m_DynamicString()
,	m_Mesh( NULL )
,	m_FontPrintFlags( FONT_PRINT_LEFT )
,	m_WrapWidth( 0.0f )
,	m_HasDropShadow( false )
,	m_DropShadowOffset()
,	m_DropShadowColor( ARGB_TO_COLOR( 255, 0, 0, 0 ) ) 
,	m_DropShadowMesh( NULL )
{
}

UIWidgetText::UIWidgetText( const SimpleString& DefinitionName )
:	m_Font( NULL )
,	m_IsLiteral( false )
,	m_IsDynamicPosition( false )
,	m_String()
,	m_DynamicString()
,	m_Mesh( NULL )
,	m_FontPrintFlags( FONT_PRINT_LEFT )
,	m_WrapWidth( 0.0f )
,	m_HasDropShadow( false )
,	m_DropShadowOffset()
,	m_DropShadowColor( ARGB_TO_COLOR( 255, 0, 0, 0 ) )
,	m_DropShadowMesh( NULL )
{
	InitializeFromDefinition( DefinitionName );
}

UIWidgetText::~UIWidgetText()
{
	SafeDelete( m_Mesh );
	SafeDelete( m_DropShadowMesh );
}

void UIWidgetText::UpdateRenderPosition()
{
	ASSERT( m_Mesh );

	m_Mesh->m_Location = Vector( m_TopLeft.x, 0.0f, m_TopLeft.y );
	m_Mesh->m_AABB.m_Min.x += m_TopLeft.x;
	m_Mesh->m_AABB.m_Min.z += m_TopLeft.y;
	m_Mesh->m_AABB.m_Max.x += m_TopLeft.x;
	m_Mesh->m_AABB.m_Max.z += m_TopLeft.y;

	if( m_HasDropShadow )
	{
		ASSERT( m_DropShadowMesh );

		m_DropShadowMesh->m_Location		= m_Mesh->m_Location;
		m_DropShadowMesh->m_Location.x		+= m_DropShadowOffset.x;
		m_DropShadowMesh->m_Location.z		+= m_DropShadowOffset.y;

		m_DropShadowMesh->m_AABB			= m_Mesh->m_AABB;
		m_DropShadowMesh->m_AABB.m_Min.x	+= m_DropShadowOffset.x;
		m_DropShadowMesh->m_AABB.m_Min.z	+= m_DropShadowOffset.y;
		m_DropShadowMesh->m_AABB.m_Max.x	+= m_DropShadowOffset.x;
		m_DropShadowMesh->m_AABB.m_Max.z	+= m_DropShadowOffset.y;
	}
}

void UIWidgetText::UpdateRender()
{
	// This will fail if UpdateRender is called between submitting UI meshes to the renderer and actually
	// rendering the scene. It may actually be safe if this widget was not submitted to the renderer.
	DEBUGASSERT( m_UIManager->DEBUGIsSafeToUpdateRender( m_LastRenderedTime ) );

	SafeDelete( m_Mesh );
	SafeDelete( m_DropShadowMesh );

	IRenderer* pRenderer = m_UIManager->GetRenderer();

	m_Mesh = m_Font->Print( m_String.CStr(), SRect( 0.0f, 0.0f, m_WrapWidth, 0.0f ), m_FontPrintFlags );
	ASSERT( m_Mesh );

	m_Mesh->m_Location = Vector( m_TopLeft.x, 0.0f, m_TopLeft.y );
	m_Mesh->m_AABB.m_Min.x += m_TopLeft.x;
	m_Mesh->m_AABB.m_Min.z += m_TopLeft.y;
	m_Mesh->m_AABB.m_Max.x += m_TopLeft.x;
	m_Mesh->m_AABB.m_Max.z += m_TopLeft.y;

	m_Mesh->SetMaterialDefinition( "Material_HUD", pRenderer );
	m_Mesh->SetMaterialFlags( m_RenderInWorld ? MAT_INWORLDHUD : MAT_HUD );
	m_Mesh->m_ConstantColor = m_Color;

	if( m_HasDropShadow )
	{
		m_DropShadowMesh = new Mesh;
		m_DropShadowMesh->Initialize( m_Mesh->m_VertexBuffer, m_Mesh->m_VertexDeclaration, m_Mesh->m_IndexBuffer, NULL );
		m_DropShadowMesh->m_Material		= m_Mesh->m_Material;

		m_DropShadowMesh->m_Location		= m_Mesh->m_Location;
		m_DropShadowMesh->m_Location.x		+= m_DropShadowOffset.x;
		m_DropShadowMesh->m_Location.z		+= m_DropShadowOffset.y;

		m_DropShadowMesh->m_AABB			= m_Mesh->m_AABB;
		m_DropShadowMesh->m_AABB.m_Min.x	+= m_DropShadowOffset.x;
		m_DropShadowMesh->m_AABB.m_Min.z	+= m_DropShadowOffset.y;
		m_DropShadowMesh->m_AABB.m_Max.x	+= m_DropShadowOffset.x;
		m_DropShadowMesh->m_AABB.m_Max.z	+= m_DropShadowOffset.y;

		m_DropShadowMesh->m_ConstantColor	= m_DropShadowColor;
	}
}

void UIWidgetText::Render( bool HasFocus )
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
		if( m_HasDropShadow )
		{
			m_UIManager->GetRenderer()->AddMesh( m_DropShadowMesh );
		}

		m_UIManager->GetRenderer()->AddMesh( m_Mesh );
	}
}

void UIWidgetText::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	UIWidget::InitializeFromDefinition( DefinitionName );

	MAKEHASH( m_Archetype );
	MAKEHASH( DefinitionName );

	STATICHASH( Font );
	m_Font = m_UIManager->GetRenderer()->GetFontManager()->GetFont( ConfigManager::GetArchetypeString( sFont, sm_Archetype, DEFAULT_FONT, sDefinitionName ) );

	// If LoadString is false, we're expecting to dynamically set the string in code somewhere
	STATICHASH( LoadString );
	if( ConfigManager::GetArchetypeBool( sLoadString, sm_Archetype, true, sDefinitionName ) )
	{
		STATICHASH( String );
		STATICHASH( DynamicString );

		STATICHASH( IsLiteral );
		m_IsLiteral = ConfigManager::GetArchetypeBool( sIsLiteral, sm_Archetype, false, sDefinitionName );
		if( m_IsLiteral )
		{
			m_String		= ConfigManager::GetArchetypeString( sString, sm_Archetype, "", sDefinitionName );
			m_DynamicString	= ConfigManager::GetArchetypeString( sDynamicString, sm_Archetype, "", sDefinitionName );
		}
		else
		{
			m_String		= ConfigManager::GetLocalizedString( ConfigManager::GetArchetypeHash( sString, sm_Archetype, "", sDefinitionName ), "" );
			m_DynamicString	= ConfigManager::GetLocalizedString( ConfigManager::GetArchetypeHash( sDynamicString, sm_Archetype, "", sDefinitionName ), "" );
		}
	}

	STATICHASH( IsDynamicPosition );
	m_IsDynamicPosition = ConfigManager::GetArchetypeBool( sIsDynamicPosition, sm_Archetype, false, sDefinitionName );

	STATICHASH( Alignment );
	GetFontPrintFlags( ConfigManager::GetArchetypeString( sAlignment, sm_Archetype, "", sDefinitionName ) );

	STATICHASH( HasDropShadow );
	m_HasDropShadow = ConfigManager::GetArchetypeBool( sHasDropShadow, sm_Archetype, false, sDefinitionName );

	STATICHASH( DropShadowColorR );
	m_DropShadowColor.r = ConfigManager::GetArchetypeFloat( sDropShadowColorR, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DropShadowColorG );
	m_DropShadowColor.g = ConfigManager::GetArchetypeFloat( sDropShadowColorG, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DropShadowColorB );
	m_DropShadowColor.b = ConfigManager::GetArchetypeFloat( sDropShadowColorB, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DropShadowColorA );
	m_DropShadowColor.a = ConfigManager::GetArchetypeFloat( sDropShadowColorA, sm_Archetype, 1.0f, sDefinitionName );

	STATICHASH( DisplayWidth );
	const float fDisplayWidth = static_cast<float>( ConfigManager::GetInt( sDisplayWidth ) );

	STATICHASH( DisplayHeight );
	const float fDisplayHeight = static_cast<float>( ConfigManager::GetInt( sDisplayHeight ) );

	STATICHASH( WrapPixelWidth );
	STATICHASH( WrapScreenWidth );
	m_WrapWidth =
		Pick(
			ConfigManager::GetArchetypeFloat( sWrapPixelWidth, sm_Archetype, 0.0f, sDefinitionName ),
			fDisplayWidth * ConfigManager::GetArchetypeFloat( sWrapScreenWidth, sm_Archetype, 0.0f, sDefinitionName ) );

	STATICHASH( DropShadowOffsetPixelX );
	STATICHASH( DropShadowOffsetScreenX );
	m_DropShadowOffset.x =
		Pick(
			ConfigManager::GetArchetypeFloat( sDropShadowOffsetPixelX, sm_Archetype, 0.0f, sDefinitionName ),
			fDisplayWidth * ConfigManager::GetArchetypeFloat( sDropShadowOffsetScreenX, sm_Archetype, 0.0f, sDefinitionName ) );

	STATICHASH( DropShadowOffsetPixelY );
	STATICHASH( DropShadowOffsetScreenY );
	m_DropShadowOffset.y =
		Pick(
			ConfigManager::GetArchetypeFloat( sDropShadowOffsetPixelY, sm_Archetype, 0.0f, sDefinitionName ),
			fDisplayHeight * ConfigManager::GetArchetypeFloat( sDropShadowOffsetScreenY, sm_Archetype, 0.0f, sDefinitionName ) );

	UpdatePosition();

	UpdateRender();
}

void UIWidgetText::GetBounds( SRect& OutBounds )
{
	OutBounds = SRect( m_Mesh->m_AABB.m_Min.x, m_Mesh->m_AABB.m_Min.z, m_Mesh->m_AABB.m_Max.x, m_Mesh->m_AABB.m_Max.z );
}

/*virtual*/ float UIWidgetText::GetWidth()
{
	return m_Mesh->m_AABB.m_Max.x - m_Mesh->m_AABB.m_Min.x;
}

/*virtual*/ float UIWidgetText::GetHeight()
{
	return m_Mesh->m_AABB.m_Max.y - m_Mesh->m_AABB.m_Min.y;
}

void UIWidgetText::Refresh()
{
	UIWidget::Refresh();

	if( m_DynamicString != "" )
	{
		const SimpleString	ResolvedDynamicString	= StringManager::ParseConfigString( StringManager::ESL_Transient, m_DynamicString.CStr() );
		const bool			StringChanged			= ( ResolvedDynamicString != m_String );

		m_String = ResolvedDynamicString;

		if( StringChanged )
		{
			if( m_IsDynamicPosition )
			{
				UpdatePosition();
				UpdateRenderPosition();
			}

			UpdateRender();
		}
	}
}

/*virtual*/ void UIWidgetText::Tick( const float DeltaTime )
{
	XTRACE_FUNCTION;

	if( !m_Velocity.IsZero() )
	{
		m_TopLeft += m_Velocity * DeltaTime;

		if( m_ClampToPixelGrid )
		{
			// Floor instead of round so we don't mess with the velocity.
			// (Essentially the same as subtracting the half pixel offset
			// before rounding, since it just adds 0.5.)
			const float PixelGridOffset = 0.5f - GetPixelGridOffset();
			m_TopLeft.x = Floor( m_TopLeft.x ) + PixelGridOffset;
			m_TopLeft.y = Floor( m_TopLeft.y ) + PixelGridOffset;
		}

		UpdateRenderPosition();
	}
}

const SimpleString& UIWidgetText::GetString()
{
	return ( m_String != "" ) ? m_String : m_DynamicString;
}

void UIWidgetText::UpdatePosition()
{
	MAKEHASH( m_Archetype );
	MAKEHASH( m_Name );

	STATICHASH( DisplayWidth );
	const float DisplayWidth	= ConfigManager::GetFloat( sDisplayWidth );
	const float ParentWidth		= m_OriginParent ? m_OriginParent->GetWidth() : DisplayWidth;
	const float ParentX			= m_OriginParent ? Ceiling( m_OriginParent->GetX() ) : 0.0f;

	STATICHASH( DisplayHeight );
	const float DisplayHeight	= ConfigManager::GetFloat( sDisplayHeight );
	const float ParentHeight	= m_OriginParent ? m_OriginParent->GetHeight() : DisplayHeight;
	const float ParentY			= m_OriginParent ? Ceiling( m_OriginParent->GetY() ) : 0.0f;

	STATICHASH( PixelX );
	STATICHASH( PixelOffsetX );
	STATICHASH( ScreenX );
	float X =
		ConfigManager::GetArchetypeFloat( sPixelOffsetX, sm_Archetype, 0.0f, sm_Name ) +
		Pick(
			ConfigManager::GetArchetypeFloat( sPixelX, sm_Archetype, 0.0f, sm_Name ),
			ParentWidth * ConfigManager::GetArchetypeFloat( sScreenX, sm_Archetype, 0.0f, sm_Name ) );
	
	STATICHASH( PixelY );
	STATICHASH( PixelOffsetY );
	STATICHASH( ScreenY );
	float Y =
		ConfigManager::GetArchetypeFloat( sPixelOffsetY, sm_Archetype, 0.0f, sm_Name ) +
		Pick(
			ConfigManager::GetArchetypeFloat( sPixelY, sm_Archetype, 0.0f, sm_Name ),
			ParentHeight * ConfigManager::GetArchetypeFloat( sScreenY, sm_Archetype, 0.0f, sm_Name ) );

	// Get dimensions so we can do different origins
	Array<STypesetGlyph> UnusedTypesetting;
	Vector2 Dimensions;
	Array<unicode_t> CodePoints;
	GetString().UTF8ToUnicode( CodePoints );
	m_Font->Arrange( CodePoints, SRect( 0.0f, 0.0f, m_WrapWidth, 0.0f ), UnusedTypesetting, Dimensions );

	AdjustDimensionsToParent( X, Y, Dimensions.x, Dimensions.y, ParentX, ParentY, ParentWidth, ParentHeight );
	GetPositionFromOrigin( X, Y, Dimensions.x, Dimensions.y );

	STATICHASH( ClampToPixelGrid );
	m_ClampToPixelGrid = ConfigManager::GetArchetypeBool( sClampToPixelGrid, sm_Archetype, true, sm_Name );
	if( m_ClampToPixelGrid )
	{
		// Add 0.5 to compensate for font UVs being on half pixel intervals.
		m_TopLeft.x = Round( m_TopLeft.x ) + 0.5f;
		m_TopLeft.y = Round( m_TopLeft.y ) + 0.5f;
	}

	// Offset to properly align on pixel grid.
	const float PixelGridOffset = GetPixelGridOffset();
	m_TopLeft.x -= PixelGridOffset;
	m_TopLeft.y -= PixelGridOffset;
}

void UIWidgetText::GetFontPrintFlags( const SimpleString& Alignment )
{
	if( Alignment == "Right" )
	{
		m_FontPrintFlags = FONT_PRINT_RIGHT;
	}
	else if( Alignment == "Center" )
	{
		m_FontPrintFlags = FONT_PRINT_CENTER;
	}
#if BUILD_DEV
	else
	{
		ASSERT( m_FontPrintFlags == FONT_PRINT_LEFT );
	}
#endif
}