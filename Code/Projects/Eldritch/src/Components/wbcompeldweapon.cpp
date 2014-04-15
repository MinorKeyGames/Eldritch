#include "core.h"
#include "wbcompeldweapon.h"
#include "configmanager.h"
#include "wbevent.h"
#include "wbcompeldtransform.h"
#include "wbcompeldammobag.h"
#include "Components/wbcompowner.h"
#include "mathcore.h"

WBCompEldWeapon::WBCompEldWeapon()
:	m_Ammo()
,	m_WeaponIcon()
{
}

WBCompEldWeapon::~WBCompEldWeapon()
{
}

/*virtual*/ void WBCompEldWeapon::InitializeFromDefinition( const SimpleString& DefinitionName )
{
	Super::InitializeFromDefinition( DefinitionName );

	MAKEHASH( DefinitionName );

	STATICHASH( Ammo );
	m_Ammo = ConfigManager::GetInheritedHash( sAmmo, HashedString::NullString, sDefinitionName );

	STATICHASH( WeaponIcon );
	m_WeaponIcon = ConfigManager::GetInheritedString( sWeaponIcon, "", sDefinitionName );
}

/*virtual*/ void WBCompEldWeapon::AddContextToEvent( WBEvent& Event ) const
{
	Super::AddContextToEvent( Event );

	if( m_Ammo != HashedString::NullString )
	{
		WBEntity* const pOwnerEntity = WBCompOwner::GetTopmostOwner( GetEntity() );
		ASSERT( pOwnerEntity );

		WBCompEldAmmoBag* const pAmmoBag = GET_WBCOMP( pOwnerEntity, EldAmmoBag );
		if( pAmmoBag )
		{
			WB_SET_CONTEXT( Event, Bool, HasAmmo, pAmmoBag->HasAmmo( m_Ammo ) );
		}
	}
}