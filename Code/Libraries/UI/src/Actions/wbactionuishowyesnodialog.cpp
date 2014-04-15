#include "core.h"
#include "wbactionuishowyesnodialog.h"
#include "configmanager.h"
#include "wbeventmanager.h"
#include "wbactionfactory.h"

WBActionUIShowYesNoDialog::WBActionUIShowYesNoDialog()
:	m_YesNoString()
,	m_YesActions()
{
}

WBActionUIShowYesNoDialog::~WBActionUIShowYesNoDialog()
{
	FOR_EACH_ARRAY( YesActionIter, m_YesActions, WBAction* )
	{
		WBAction* pYesAction = YesActionIter.GetValue();
		SafeDelete( pYesAction );
	}
}

/*virtual*/ void WBActionUIShowYesNoDialog::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	WBAction::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( YesNoString );
	m_YesNoString = ConfigManager::GetString( sYesNoString, "", sDefinitionName );

	STATICHASH( NumYesActions );
	const uint NumYesActions = ConfigManager::GetInt( sNumYesActions, 0, sDefinitionName );
	for( uint YesActionIndex = 0; YesActionIndex < NumYesActions; ++YesActionIndex )
	{
		const SimpleString YesAction = ConfigManager::GetSequenceString( "YesAction%d", YesActionIndex, "", DefinitionName );
		WBAction* const pYesAction = WBActionFactory::Create( YesAction );
		if( pYesAction )
		{
			m_YesActions.PushBack( pYesAction );
		}
	}
}

/*virtual*/ void WBActionUIShowYesNoDialog::Execute()
{
	// We can't push directly to the UI stack because we don't know anything about Framework3D
	// or whoever else might own a UI manager and stack. Instead, use Workbench events.

	WB_MAKE_EVENT( ShowYesNoDialog, NULL );
	WB_SET_AUTO( ShowYesNoDialog, Hash, YesNoString, m_YesNoString );
	WB_SET_AUTO( ShowYesNoDialog, Pointer, YesActions, &m_YesActions );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), ShowYesNoDialog, NULL );
}