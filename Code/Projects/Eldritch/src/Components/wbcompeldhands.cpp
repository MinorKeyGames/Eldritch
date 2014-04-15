#include "core.h"
#include "wbcompeldhands.h"
#include "wbcompeldweapon.h"
#include "wbeventmanager.h"
#include "hashedstring.h"
#include "wbcompeldinventory.h"
#include "idatastream.h"
#include "inputsystem.h"

WBCompEldHands::WBCompEldHands()
:	m_HideHandsRefs( 0 )
{
	STATIC_HASHED_STRING( OnWorldLoaded );
	GetEventManager()->AddObserver( sOnWorldLoaded, this );
}

WBCompEldHands::~WBCompEldHands()
{
	WBEventManager* const pEventManager = GetEventManager();
	if( pEventManager )
	{
		STATIC_HASHED_STRING( OnWorldLoaded );
		pEventManager->RemoveObserver( sOnWorldLoaded, this );
	}
}

/*virtual*/ void WBCompEldHands::HandleEvent( const WBEvent& Event )
{
	XTRACE_FUNCTION;

	Super::HandleEvent( Event );

	STATIC_HASHED_STRING( OnWorldLoaded );
	STATIC_HASHED_STRING( OnItemEquipped );
	STATIC_HASHED_STRING( OnItemUnequipped );
	STATIC_HASHED_STRING( OnItemsSwapped );
	STATIC_HASHED_STRING( UseRightHand );
	STATIC_HASHED_STRING( UseLeftHand );
	STATIC_HASHED_STRING( ShowHands );
	STATIC_HASHED_STRING( HideHands );
	STATIC_HASHED_STRING( PlayHandAnim );
	STATIC_HASHED_STRING( SetHandMeshes );

	const HashedString EventName = Event.GetEventName();
	if( EventName == sOnWorldLoaded )
	{
		if( GetItemInRightHand() == GetWeapon() )
		{
			WB_MAKE_EVENT( OnWeaponEquipped, GetEntity() );
			WB_SET_AUTO( OnWeaponEquipped, Entity, Weapon, GetWeapon() );
			WB_DISPATCH_EVENT( GetEventManager(), OnWeaponEquipped, GetEntity() );
		}

		RestoreHandAnimations();

		UpdateWeaponHUD();
	}
	else if( EventName == sOnItemEquipped )
	{
		STATIC_HASHED_STRING( Item );
		WBEntity* const pItem = Event.GetEntity( sItem );
		ASSERT( pItem );

		// Hide fists and show ammo when we equip a weapon
		if( pItem == GetWeapon() )
		{
			WB_MAKE_EVENT( Hide, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), Hide, GetFists() );

			WB_MAKE_EVENT( OnWeaponEquipped, GetEntity() );
			WB_SET_AUTO( OnWeaponEquipped, Entity, Weapon, pItem );
			WB_DISPATCH_EVENT( GetEventManager(), OnWeaponEquipped, GetEntity() );

			ShowWeaponHUD();
		}
		
		if( pItem == GetItemInRightHand() ||
			pItem == GetItemInLeftHand() )
		{
			AddAnimationsToHand( pItem );
		}

		if( pItem == GetWeaponAlt() )
		{
			// When traveling to a new world and spawning an item in the alt slot, immediately hide it
			WB_MAKE_EVENT( Hide, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), Hide, GetWeaponAlt() );

			ShowWeaponAltHUD();
		}
	}
	else if( EventName == sOnItemUnequipped )
	{
		// Show fists when we unequip a weapon
		STATIC_HASHED_STRING( Item );
		WBEntity* const pItem = Event.GetEntity( sItem );
		if( pItem == GetWeapon() )
		{
			WB_MAKE_EVENT( Show, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), Show, GetFists() );

			WB_MAKE_EVENT( OnWeaponUnequipped, GetEntity() );
			WB_DISPATCH_EVENT( GetEventManager(), OnWeaponUnequipped, GetEntity() );

			// Revert to the fists animations
			AddAnimationsToHand( GetFists(), EH_Right );

			// HACK: Play the idle animation. I could add an event for "restored equip focus"
			// or whatever and hook this up in data, but eh. That somehow feels worse.
			STATIC_HASHED_STRING( Idle );
			PlayAnimation( GetFists(), sIdle, EH_Right );

			HideWeaponHUD();
		}
	}
	else if( EventName == sOnItemsSwapped )
	{
		// HACK, since the only things we swap (currently) are Weapon/WeaponAlt

		// Hide the alt weapon, show the right hand item
		WB_MAKE_EVENT( Hide, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), Hide, GetWeaponAlt() );

		WB_MAKE_EVENT( Show, GetEntity() );
		WB_DISPATCH_EVENT( GetEventManager(), Show, GetItemInRightHand() );

		AddAnimationsToHand( GetItemInRightHand() );

		STATIC_HASHED_STRING( Idle );
		PlayAnimation( GetItemInRightHand(), sIdle, EH_Right );

		UpdateWeaponHUD();

		WB_MAKE_EVENT( OnWeaponEquipped, GetEntity() );
		WB_SET_AUTO( OnWeaponEquipped, Entity, Weapon, GetItemInRightHand() );
		WB_DISPATCH_EVENT( GetEventManager(), OnWeaponEquipped, GetEntity() );
	}
	else if( EventName == sUseRightHand )
	{
		WBEntity* const pWeapon = GetItemInRightHand();
		if( pWeapon )
		{
			STATIC_HASHED_STRING( InputEdge );
			const int InputEdge = Event.GetInt( sInputEdge );

			{
				WB_MAKE_EVENT( Use, GetEntity() );
				WB_SET_AUTO( Use, Int, InputEdge, InputEdge );
				WB_DISPATCH_EVENT( GetEventManager(), Use, pWeapon );
			}
		}
	}
	else if( EventName == sUseLeftHand )
	{
		WBEntity* const pPower = GetItemInLeftHand();
		if( pPower )
		{
			STATIC_HASHED_STRING( InputEdge );
			const int InputEdge = Event.GetInt( sInputEdge );

			{
				WB_MAKE_EVENT( Use, GetEntity() );
				WB_SET_AUTO( Use, Int, InputEdge, InputEdge );
				WB_DISPATCH_EVENT( GetEventManager(), Use, pPower );
			}
		}
	}
	else if( EventName == sShowHands )
	{
		DecrementHideHandsRefs();
	}
	else if( EventName == sHideHands )
	{
		IncrementHideHandsRefs();
	}
	else if( EventName == sPlayHandAnim )
	{
		STATIC_HASHED_STRING( AnimatingEntity );
		WBEntity* const pAnimatingEntity = Event.GetEntity( sAnimatingEntity );

		STATIC_HASHED_STRING( AnimationName );
		const HashedString AnimationName = Event.GetHash( sAnimationName );

		// Don't play hand anim if we're restoring the alternate weapon
		if( pAnimatingEntity == GetItemInRightHand() || pAnimatingEntity == GetItemInLeftHand() )
		{
			const EHand Hand = GetHandEnum( pAnimatingEntity );
			PlayAnimation( pAnimatingEntity, AnimationName, Hand );
		}
	}
	else if( EventName == sSetHandMeshes )
	{
		STATIC_HASHED_STRING( LeftHandMesh );
		const SimpleString LeftHandMesh = Event.GetString( sLeftHandMesh );

		STATIC_HASHED_STRING( LeftHandTexture );
		const SimpleString LeftHandTexture = Event.GetString( sLeftHandTexture );

		STATIC_HASHED_STRING( RightHandMesh );
		const SimpleString RightHandMesh = Event.GetString( sRightHandMesh );

		STATIC_HASHED_STRING( RightHandTexture );
		const SimpleString RightHandTexture = Event.GetString( sRightHandTexture );

		SetHandMeshes( LeftHandMesh, LeftHandTexture, RightHandMesh, RightHandTexture );
		RestoreHandAnimations();
	}
}

// HACKHACK: This whole thing is way too content-aware.
void WBCompEldHands::UpdateWeaponHUD() const
{
	if( GetWeapon() )
	{
		ShowWeaponHUD();
	}
	else
	{
		HideWeaponHUD();
	}

	if( GetWeaponAlt() )
	{
		ShowWeaponAltHUD();
	}
	else
	{
		HideWeaponAltHUD();
	}
}

void WBCompEldHands::ShowWeaponHUD() const
{
	WBEntity* const pWeapon = GetWeapon();
	ASSERT( pWeapon );

	WBCompEldWeapon* const pWeaponComponent = GET_WBCOMP( pWeapon, EldWeapon );
	ASSERT( pWeaponComponent );

	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( WeaponIcon );

	{
		WB_MAKE_EVENT( SetWidgetImage, NULL );
		WB_SET_AUTO( SetWidgetImage, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetImage, Hash, Widget, sWeaponIcon );
		WB_SET_AUTO( SetWidgetImage, Hash, Image, pWeaponComponent->GetWeaponIcon() );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetImage, NULL );
	}

	{
		WB_MAKE_EVENT( SetWidgetHidden, NULL );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sWeaponIcon );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, false );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetHidden, NULL );
	}
}

void WBCompEldHands::ShowWeaponAltHUD() const
{
	WBEntity* const pWeapon = GetWeaponAlt();
	ASSERT( pWeapon );

	WBCompEldWeapon* const pWeaponComponent = GET_WBCOMP( pWeapon, EldWeapon );
	ASSERT( pWeaponComponent );

	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( WeaponAltIcon );

	{
		WB_MAKE_EVENT( SetWidgetImage, NULL );
		WB_SET_AUTO( SetWidgetImage, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetImage, Hash, Widget, sWeaponAltIcon );
		WB_SET_AUTO( SetWidgetImage, Hash, Image, pWeaponComponent->GetWeaponIcon() );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetImage, NULL );
	}

	{
		WB_MAKE_EVENT( SetWidgetHidden, NULL );
		WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
		WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sWeaponAltIcon );
		WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, false );
		WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetHidden, NULL );
	}
}

void WBCompEldHands::HideWeaponHUD() const
{
	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( WeaponIcon );

	WB_MAKE_EVENT( SetWidgetHidden, NULL );
	WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
	WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sWeaponIcon );
	WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, true );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetHidden, NULL );
}

void WBCompEldHands::HideWeaponAltHUD() const
{
	STATIC_HASHED_STRING( HUD );
	STATIC_HASHED_STRING( WeaponAltIcon );

	WB_MAKE_EVENT( SetWidgetHidden, NULL );
	WB_SET_AUTO( SetWidgetHidden, Hash, Screen, sHUD );
	WB_SET_AUTO( SetWidgetHidden, Hash, Widget, sWeaponAltIcon );
	WB_SET_AUTO( SetWidgetHidden, Bool, Hidden, true );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), SetWidgetHidden, NULL );
}

void WBCompEldHands::IncrementHideHandsRefs()
{
	if( ++m_HideHandsRefs == 1 )
	{
		HideHands();
	}
}

void WBCompEldHands::DecrementHideHandsRefs()
{
	if( m_HideHandsRefs > 0 )
	{
		if( --m_HideHandsRefs == 0 )
		{
			ShowHands();
		}
	}
}

void WBCompEldHands::HideHands() const
{
	WB_MAKE_EVENT( Hide, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), Hide, GetRightHand() );
	WB_DISPATCH_EVENT( GetEventManager(), Hide, GetLeftHand() );
	WB_DISPATCH_EVENT( GetEventManager(), Hide, GetItemInRightHand() );
	WB_DISPATCH_EVENT( GetEventManager(), Hide, GetItemInLeftHand() );
}

void WBCompEldHands::ShowHands() const
{
	WB_MAKE_EVENT( Show, GetEntity() );
	WB_DISPATCH_EVENT( GetEventManager(), Show, GetRightHand() );
	WB_DISPATCH_EVENT( GetEventManager(), Show, GetLeftHand() );
	WB_DISPATCH_EVENT( GetEventManager(), Show, GetItemInRightHand() );
	WB_DISPATCH_EVENT( GetEventManager(), Show, GetItemInLeftHand() );
}

void WBCompEldHands::PlayAnimation( WBEntity* const pAnimatingEntity, const HashedString& AnimationName, const EHand Hand ) const
{
	if( !pAnimatingEntity )
	{
		return;
	}

	WB_MAKE_EVENT( PlayAnim, GetEntity() );
	WB_SET_AUTO( PlayAnim, Hash, AnimationName, AnimationName );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), PlayAnim, pAnimatingEntity );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), PlayAnim, GetHand( Hand ) );
}

void WBCompEldHands::AddAnimationsToHand( WBEntity* const pItem ) const
{
	const EHand Hand = GetHandEnum( pItem );
	AddAnimationsToHand( pItem, Hand );
}

void WBCompEldHands::AddAnimationsToHand( WBEntity* const pItem, const EHand Hand ) const
{
	if( !pItem )
	{
		return;
	}

	if( pItem == GetRightHand() || pItem == GetLeftHand() )
	{
		return;
	}

	WB_MAKE_EVENT( CopyAnimations, GetEntity() );
	WB_SET_AUTO( CopyAnimations, Entity, SourceEntity, pItem );
	WB_SET_AUTO( CopyAnimations, Bool, SuppressAnimEvents, true );
	WB_DISPATCH_EVENT( WBWorld::GetInstance()->GetEventManager(), CopyAnimations, GetHand( Hand ) );
}

void WBCompEldHands::SetHandMeshes(
	const SimpleString& LeftHandMesh,
	const SimpleString& LeftHandTexture,
	const SimpleString& RightHandMesh,
	const SimpleString& RightHandTexture ) const
{
	{
		WB_MAKE_EVENT( SetMesh, GetEntity() );
		WB_SET_AUTO( SetMesh, Hash, Mesh, LeftHandMesh );
		WB_SET_AUTO( SetMesh, Hash, Texture, LeftHandTexture );
		WB_DISPATCH_EVENT( GetEventManager(), SetMesh, GetLeftHand() );
	}

	{
		WB_MAKE_EVENT( SetMesh, GetEntity() );
		WB_SET_AUTO( SetMesh, Hash, Mesh, RightHandMesh );
		WB_SET_AUTO( SetMesh, Hash, Texture, RightHandTexture );
		WB_DISPATCH_EVENT( GetEventManager(), SetMesh, GetRightHand() );
	}
}

void WBCompEldHands::RestoreHandAnimations() const
{
	// Restore hand animations from whatever is equipped.
	AddAnimationsToHand( GetItemInRightHand(), EH_Right );
	AddAnimationsToHand( GetItemInLeftHand(), EH_Left );

	// HACK: And play the idle animations.
	STATIC_HASHED_STRING( Idle );
	PlayAnimation( GetItemInRightHand(), sIdle, EH_Right );
	PlayAnimation( GetItemInLeftHand(), sIdle, EH_Left );
}

void WBCompEldHands::GiveItem( const SimpleString& ItemDef ) const
{
	WBCompEldInventory* const	pInventory		= GET_WBCOMP( GetEntity(), EldInventory );
	ASSERT( pInventory );

	pInventory->AddItem( ItemDef );
}

WBCompEldInventory* WBCompEldHands::GetInventory() const
{
	WBCompEldInventory* const pInventory = GET_WBCOMP( GetEntity(), EldInventory );
	ASSERT( pInventory );
	return pInventory;
}

WBCompEldHands::EHand WBCompEldHands::GetHandEnum( WBEntity* const pItem ) const
{
	if( pItem == GetItemInRightHand() )
	{
		return EH_Right;
	}
	else if( pItem == GetItemInLeftHand() )
	{
		return EH_Left;
	}
	else
	{
		WARN;
		return EH_None;
	}
}

WBEntity* WBCompEldHands::GetHand( const EHand Hand ) const
{
	if( Hand == EH_Right )
	{
		return GetRightHand();
	}
	else if( Hand == EH_Left )
	{
		return GetLeftHand();
	}
	else
	{
		WARN;
		return NULL;
	}
}

WBEntity* WBCompEldHands::GetRightHand() const
{
	STATIC_HASHED_STRING( RightHand );
	return GetInventory()->GetItem( sRightHand );
}

WBEntity* WBCompEldHands::GetLeftHand() const
{
	STATIC_HASHED_STRING( LeftHand );
	return GetInventory()->GetItem( sLeftHand );
}

WBEntity* WBCompEldHands::GetItemInRightHand() const
{
	WBEntity* const pWeapon = GetWeapon();
	return pWeapon ? pWeapon : GetFists();
}

WBEntity* WBCompEldHands::GetItemInLeftHand() const
{
	return GetPower();
}

WBEntity* WBCompEldHands::GetWeapon() const
{
	STATIC_HASHED_STRING( Weapon );
	return GetInventory()->GetItem( sWeapon );
}

WBEntity* WBCompEldHands::GetWeaponAlt() const
{
	STATIC_HASHED_STRING( WeaponAlt );
	return GetInventory()->GetItem( sWeaponAlt );
}

WBEntity* WBCompEldHands::GetPower() const
{
	STATIC_HASHED_STRING( Power );
	return GetInventory()->GetItem( sPower );
}

WBEntity* WBCompEldHands::GetFists() const
{
	STATIC_HASHED_STRING( Fists );
	return GetInventory()->GetItem( sFists );
}

#define VERSION_EMPTY			0
#define VERSION_HIDEHANDSREFS	1
#define VERSION_CURRENT			2

uint WBCompEldHands::GetSerializationSize()
{
	uint Size = 0;

	Size += 4;					// Version
	Size += 4;					// m_HideHandsRefs

	return Size;
}

void WBCompEldHands::Save( const IDataStream& Stream )
{
	Stream.WriteUInt32( VERSION_CURRENT );

	Stream.WriteInt32( m_HideHandsRefs );
}

void WBCompEldHands::Load( const IDataStream& Stream )
{
	XTRACE_FUNCTION;

	const uint Version = Stream.ReadUInt32();

	if( Version >= VERSION_HIDEHANDSREFS )
	{
		m_HideHandsRefs = Stream.ReadInt32();
	}
}