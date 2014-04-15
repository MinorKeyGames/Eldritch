#include "core.h"
#include "uiwidget-spacer.h"
#include "uimanager.h"
#include "configmanager.h"
#include "mathcore.h"
#include "3d.h"

UIWidgetSpacer::UIWidgetSpacer()
:	m_Dimensions()
{
}

UIWidgetSpacer::~UIWidgetSpacer()
{
}

// TODO: Lots of copied and pasted code in these widgets. Unify that.
void UIWidgetSpacer::InitializeFromDefinition( const SimpleString& DefinitionName )
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

	AdjustDimensionsToParent( X, Y, Width, Height, ParentX, ParentY, ParentWidth, ParentHeight );
	GetPositionFromOrigin( X, Y, Width, Height );

	STATICHASH( ClampToPixelGrid );
	if( ConfigManager::GetArchetypeBool( sClampToPixelGrid, sm_Archetype, true, sDefinitionName ) )
	{
		m_TopLeft.x = Round( m_TopLeft.x );
		m_TopLeft.y = Round( m_TopLeft.y );
	}

	m_Dimensions = Vector2( Width, Height );
}

void UIWidgetSpacer::GetBounds( SRect& OutBounds )
{
	OutBounds = SRect( m_TopLeft.x, m_TopLeft.y, m_TopLeft.x + m_Dimensions.x, m_TopLeft.y + m_Dimensions.y );
}