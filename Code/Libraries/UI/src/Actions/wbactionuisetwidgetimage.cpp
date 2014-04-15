#include "core.h"
#include "wbactionuisetwidgetimage.h"
#include "configmanager.h"
#include "wbeventmanager.h"

WBActionUISetWidgetImage::WBActionUISetWidgetImage()
:	m_ScreenName()
,	m_WidgetName()
,	m_Image()
,	m_ImagePE()
{
}

WBActionUISetWidgetImage::~WBActionUISetWidgetImage()
{
}

/*virtual*/ void WBActionUISetWidgetImage::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Screen );
	m_ScreenName = ConfigManager::GetHash( sScreen, HashedString::NullString, sDefinitionName );

	STATICHASH( Widget );
	m_WidgetName = ConfigManager::GetHash( sWidget, HashedString::NullString, sDefinitionName );

	STATICHASH( Image );
	m_Image = ConfigManager::GetHash( sImage, HashedString::NullString, sDefinitionName );

	STATICHASH( ImagePE );
	const SimpleString ImagePEDef = ConfigManager::GetString( sImagePE, "", sDefinitionName );
	m_ImagePE.InitializeFromDefinition( ImagePEDef );
}

/*virtual*/ void WBActionUISetWidgetImage::Execute()
{
	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = GetEntity();

	m_ImagePE.Evaluate( PEContext );
	const HashedString Image = ( m_ImagePE.GetType() == WBParamEvaluator::EPT_String ) ? m_ImagePE.GetString() : m_Image;

	WB_MAKE_EVENT( SetWidgetImage, NULL );
	WB_SET_AUTO( SetWidgetImage, Hash, Screen, m_ScreenName );
	WB_SET_AUTO( SetWidgetImage, Hash, Widget, m_WidgetName );
	WB_SET_AUTO( SetWidgetImage, Hash, Image, Image );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetImage, NULL );
}