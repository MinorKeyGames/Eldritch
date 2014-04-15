#include "core.h"
#include "wbcompeldhudmarker.h"
#include "wbcompeldtransform.h"
#include "wbentity.h"
#include "configmanager.h"
#include "eldritchframework.h"
#include "view.h"
#include "wbeventmanager.h"
#include "vector2.h"
#include "mathcore.h"
#include "eldritchworld.h"
#include "collisioninfo.h"

WBCompEldHUDMarker::WBCompEldHUDMarker()
:	m_UIScreenName()
,	m_UIWidgetName()
,	m_OccludedImage()
,	m_UnoccludedImage()
,	m_FalloffRadius( 0.0f )
,	m_OffsetZ( 0.0f )
{
	STATIC_HASHED_STRING( OnCameraTicked );
	GetEventManager()->AddObserver( sOnCameraTicked, this );
}

WBCompEldHUDMarker::~WBCompEldHUDMarker()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnCameraTicked );
		GetEventManager()->RemoveObserver( sOnCameraTicked, this );
	}
}

/*virtual*/ void WBCompEldHUDMarker::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( UIScreenName );
	m_UIScreenName = ConfigManager::GetInheritedHash( sUIScreenName, HashedString::NullString, sDefinitionName );

	STATICHASH( UIWidgetName );
	m_UIWidgetName = ConfigManager::GetInheritedHash( sUIWidgetName, HashedString::NullString, sDefinitionName );

	STATICHASH( OccludedImage );
	m_OccludedImage = ConfigManager::GetInheritedHash( sOccludedImage, HashedString::NullString, sDefinitionName );

	STATICHASH( UnoccludedImage );
	m_UnoccludedImage = ConfigManager::GetInheritedHash( sUnoccludedImage, m_OccludedImage, sDefinitionName );

	STATICHASH( FalloffRadius );
	m_FalloffRadius = ConfigManager::GetInheritedFloat( sFalloffRadius, 0.0f, sDefinitionName );

	STATICHASH( OffsetZ );
	m_OffsetZ = ConfigManager::GetInheritedFloat( sOffsetZ, 0.0f, sDefinitionName );
}

/*virtual*/ void WBCompEldHUDMarker::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnCameraTicked );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnCameraTicked )
	{
		UpdateMarkerPosition();
	}
}

void WBCompEldHUDMarker::UpdateMarkerPosition() const
{
	WBEntity* const				pEntity			= GetEntity();
	WBCompEldTransform* const	pTransform		= pEntity->GetTransformComponent<WBCompEldTransform>();
	EldritchWorld* const		pWorld			= GetWorld();
	const View* const			pView			= GetFramework()->GetMainView();
	Vector						Location		= pTransform->GetLocation();
	Location.z									+= m_OffsetZ;
	const Vector&				ViewLocation	= pView->m_Location;
	const Vector2				ScreenLocation	= pView->ProjectAndClipToScreen( Location );

	CollisionInfo Info;
	Info.m_CollideWorld							= true;
	Info.m_CollideEntities						= true;
	Info.m_CollidingEntity						= pEntity;
	Info.m_UserFlags							= EECF_Occlusion;
	Info.m_StopAtAnyCollision					= true;

	const bool					Occluded		= pWorld->LineCheck( ViewLocation, Location, Info );
	const float					Distance		= ( Location - ViewLocation ).Length();
	const float					Alpha			= Occluded ? Attenuate( Distance, m_FalloffRadius ) : 1.0f;

	{
		WB_MAKE_EVENT( SetWidgetImage, NULL );
		WB_SET_AUTO( SetWidgetImage, Hash, Screen, m_UIScreenName );
		WB_SET_AUTO( SetWidgetImage, Hash, Widget, m_UIWidgetName );
		WB_SET_AUTO( SetWidgetImage, Hash, Image, Occluded ? m_OccludedImage : m_UnoccludedImage );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetImage, NULL );
	}

	{
		WB_MAKE_EVENT( SetWidgetLocation, NULL );
		WB_SET_AUTO( SetWidgetLocation, Hash, Screen, m_UIScreenName );
		WB_SET_AUTO( SetWidgetLocation, Hash, Widget, m_UIWidgetName );
		WB_SET_AUTO( SetWidgetLocation, Float, X, ScreenLocation.x );
		WB_SET_AUTO( SetWidgetLocation, Float, Y, ScreenLocation.y );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetLocation, NULL );
	}

	{
		WB_MAKE_EVENT( SetWidgetAlpha, NULL );
		WB_SET_AUTO( SetWidgetAlpha, Hash, Screen, m_UIScreenName );
		WB_SET_AUTO( SetWidgetAlpha, Hash, Widget, m_UIWidgetName );
		WB_SET_AUTO( SetWidgetAlpha, Float, Alpha, Alpha );
		WB_DISPATCH_EVENT( GetEventManager(), SetWidgetAlpha, NULL );
	}
}