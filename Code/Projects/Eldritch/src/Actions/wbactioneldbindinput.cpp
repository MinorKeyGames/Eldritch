#include "core.h"
#include "wbactioneldbindinput.h"
#include "configmanager.h"
#include "eldritchframework.h"
#include "inputsystem.h"
#include "Common/uimanagercommon.h"
#include "uistack.h"
#include "uiscreen.h"

WBActionEldBindInput::WBActionEldBindInput()
:	m_Input()
{
}

WBActionEldBindInput::~WBActionEldBindInput()
{
}

/*virtual*/ void WBActionEldBindInput::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Input );
	m_Input = ConfigManager::GetString( sInput, "", sDefinitionName );
}

/*virtual*/ void WBActionEldBindInput::Execute()
{
	WBAction::Execute();

	EldritchFramework* const	pFramework		= EldritchFramework::GetInstance();
	ASSERT( pFramework );

	UIManager* const			pUIManager		= pFramework->GetUIManager();
	ASSERT( pUIManager );

	UIStack* const				pUIStack		= pUIManager->GetUIStack();
	ASSERT( pUIStack );

	STATIC_HASHED_STRING( BindDialog );

	UIScreen* const				pUIBindDialog	= pUIManager->GetScreen( sBindDialog );
	ASSERT( pUIBindDialog );

	InputSystem* const			pInputSystem	= pFramework->GetInputSystem();
	ASSERT( pInputSystem );

	pUIStack->Push( pUIBindDialog );

	pInputSystem->BindInput( m_Input );
}