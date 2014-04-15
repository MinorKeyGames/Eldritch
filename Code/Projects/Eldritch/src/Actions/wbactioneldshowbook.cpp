#include "core.h"
#include "wbactioneldshowbook.h"
#include "configmanager.h"
#include "eldritchframework.h"
#include "Common/uimanagercommon.h"
#include "uistack.h"
#include "uiscreen.h"
#include "Widgets/uiwidget-text.h"

WBActionEldShowBook::WBActionEldShowBook()
:	m_BookString()
,	m_BookStringPE()
,	m_IsDynamic( false )
{
}

WBActionEldShowBook::~WBActionEldShowBook()
{
}

/*virtual*/ void WBActionEldShowBook::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( BookString );
	m_BookString = ConfigManager::GetString( sBookString, "", sDefinitionName );

	STATICHASH( BookStringPE );
	const SimpleString BookStringPEDef = ConfigManager::GetString( sBookStringPE, "", sDefinitionName );
	m_BookStringPE.InitializeFromDefinition( BookStringPEDef );

	STATICHASH( IsDynamic );
	m_IsDynamic = ConfigManager::GetBool( sIsDynamic, false, sDefinitionName );
}

/*virtual*/ void WBActionEldShowBook::Execute()
{
	WBAction::Execute();

	EldritchFramework* const pFramework = EldritchFramework::GetInstance();
	ASSERT( pFramework );

	UIManager* const pUIManager = pFramework->GetUIManager();
	ASSERT( pUIManager );

	UIStack* const pUIStack = pUIManager->GetUIStack();
	ASSERT( pUIStack );

	STATIC_HASHED_STRING( BookScreen );
	UIScreen* const pBookScreen = pUIManager->GetScreen( sBookScreen );
	ASSERT( pBookScreen );

	STATIC_HASHED_STRING( BookText );
	UIWidgetText* const pBookText = pBookScreen->GetWidget<UIWidgetText>( sBookText );
	ASSERT( pBookText );

	WBParamEvaluator::SPEContext PEContext;
	PEContext.m_Entity = GetEntity();

	m_BookStringPE.Evaluate( PEContext );
	const SimpleString BookString = ( m_BookStringPE.GetType() == WBParamEvaluator::EPT_String ) ? m_BookStringPE.GetString() : m_BookString;

	MAKEHASH( BookString );

	if( m_IsDynamic )
	{
		pBookText->m_String			= "";
		pBookText->m_DynamicString	= ConfigManager::GetLocalizedString( BookString, "" );
	}
	else
	{
		pBookText->m_String			= ConfigManager::GetLocalizedString( BookString, "" );
		pBookText->m_DynamicString	= "";
	}
	pBookText->Reinitialize();

	pUIStack->Push( pBookScreen );
}