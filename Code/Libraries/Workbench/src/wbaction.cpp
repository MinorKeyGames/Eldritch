#include "core.h"
#include "wbaction.h"
#include "simplestring.h"
#include "wbactionstack.h"
#include "Components/wbcompowner.h"
#include "wbevent.h"

WBAction::WBAction()
{
}

WBAction::~WBAction()
{
}

/*virtual*/ void WBAction::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	// Do nothing
	Unused( DefinitionName );
}

/*virtual*/ void WBAction::Execute()
{
	// Do nothing
}

WBEntity* WBAction::GetEntity() const
{
	STATIC_HASHED_STRING( EventOwner );
	return WBActionStack::Top().GetEntity( sEventOwner );
}

WBEntity* WBAction::GetOwner() const
{
	return WBCompOwner::GetTopmostOwner( GetEntity() );
}