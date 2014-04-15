#include "core.h"
#include "uiscreen-yesnodialog.h"
#include "Widgets/uiwidget-text.h"
#include "configmanager.h"

UIScreenYesNoDialog::UIScreenYesNoDialog()
{
	InitializeFromDefinition( "YesNoDialog" );
}

UIScreenYesNoDialog::~UIScreenYesNoDialog() {}

void UIScreenYesNoDialog::SetParameters(
	bool							PauseGame,
	const SimpleString&				YesNoString,
	const SimpleString&				YesNoDynamicString,
	const HashedString&				YesEvent,
	const HashedString&				NoEvent,
	const SimpleString&				YesCommand,
	const SimpleString&				NoCommand,
	const SUICallback&				YesCallback,
	const SUICallback&				NoCallback,
	const Array<WBAction*>* const	pYesActions,
	const Array<WBAction*>* const	pNoActions )
{
	m_PausesGame = PauseGame;

	// HACKY: Referencing widgets directly by index
	UIWidgetText* pDialog = static_cast< UIWidgetText* >( m_RenderWidgets[1] );
	UIWidgetText* pYes = static_cast< UIWidgetText* >( m_RenderWidgets[2] );
	UIWidgetText* pNo = static_cast< UIWidgetText* >( m_RenderWidgets[3] );

	// NOTE: This bypasses the "IsLiteral" property of the widget and always uses localized
	pDialog->m_String = ConfigManager::GetLocalizedString( YesNoString, "" );
	pDialog->m_DynamicString = ConfigManager::GetLocalizedString( YesNoDynamicString, "" );
	pDialog->InitializeFromDefinition( pDialog->m_Name );

	pYes->m_EventName		= YesEvent;
	pYes->m_Command			= YesCommand;
	pYes->m_Callback		= YesCallback;
	pYes->m_OwnsActions		= false;
	if( pYesActions )
	{
		pYes->m_Actions		= *pYesActions;
	}
	else
	{
		pYes->m_Actions.Clear();
	}

	pNo->m_EventName		= NoEvent;
	pNo->m_Command			= NoCommand;
	pNo->m_Callback			= NoCallback;
	pNo->m_OwnsActions		= false;
	if( pNoActions )
	{
		pNo->m_Actions		= *pNoActions;
	}
	else
	{
		pNo->m_Actions.Clear();
	}
}