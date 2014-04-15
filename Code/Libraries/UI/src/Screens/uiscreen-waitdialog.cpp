#include "core.h"
#include "uiscreen-waitdialog.h"
#include "Widgets/uiwidget-text.h"
#include "configmanager.h"

UIScreenWaitDialog::UIScreenWaitDialog()
{
	InitializeFromDefinition( "WaitDialog" );
}

UIScreenWaitDialog::~UIScreenWaitDialog()
{
}

UIScreen::ETickReturn UIScreenWaitDialog::Tick( float DeltaTime, bool HasFocus )
{
	XTRACE_FUNCTION;

	ETickReturn RetVal = UIScreen::Tick( DeltaTime, HasFocus );

	if( RetVal == ETR_Close )
	{
		// Don't allow backing out of this screen
		RetVal = ETR_None;
	}

	return RetVal;
}

void UIScreenWaitDialog::SetParameters(
	bool				PauseGame,
	const SimpleString&	WaitString,
	const SimpleString&	WaitDynamicString )
{
	m_PausesGame = PauseGame;

	// HACKY: Referencing widgets directly by index
	UIWidgetText* pDialog = static_cast< UIWidgetText* >( m_RenderWidgets[1] );

	// NOTE: This bypasses the "IsLiteral" property of the widget and always uses localized
	pDialog->m_String = ConfigManager::GetLocalizedString( WaitString, "" );
	pDialog->m_DynamicString = ConfigManager::GetLocalizedString( WaitDynamicString, "" );
	pDialog->InitializeFromDefinition( pDialog->m_Name );
}