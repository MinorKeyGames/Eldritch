#ifndef WBCOMPELDHANDS_H
#define WBCOMPELDHANDS_H

#include "wbeldritchcomponent.h"

class WBCompEldInventory;

class WBCompEldHands : public WBEldritchComponent
{
public:
	WBCompEldHands();
	virtual ~WBCompEldHands();

	DEFINE_WBCOMP( EldHands, WBEldritchComponent );

	virtual int		GetTickOrder() { return ETO_NoTick; }

	virtual void	HandleEvent( const WBEvent& Event );

	virtual uint	GetSerializationSize();
	virtual void	Save( const IDataStream& Stream );
	virtual void	Load( const IDataStream& Stream );

private:
	enum EHand
	{
		EH_None,
		EH_Left,
		EH_Right,
	};

	void				IncrementHideHandsRefs();
	void				DecrementHideHandsRefs();
	void				HideHands() const;
	void				ShowHands() const;

	void				PlayAnimation( WBEntity* const pAnimatingEntity, const HashedString& AnimationName, const EHand Hand ) const;
	void				AddAnimationsToHand( WBEntity* const pItem ) const;
	void				AddAnimationsToHand( WBEntity* const pItem, const EHand Hand ) const;
	void				SetHandMeshes(
							const SimpleString& LeftHandMesh,
							const SimpleString& LeftHandTexture,
							const SimpleString& RightHandMesh,
							const SimpleString& RightHandTexture ) const;
	void				RestoreHandAnimations() const;

	// HACK: Update HUD from weapon and alt weapon
	void				UpdateWeaponHUD() const;
	void				ShowWeaponHUD() const;
	void				ShowWeaponAltHUD() const;
	void				HideWeaponHUD() const;
	void				HideWeaponAltHUD() const;

	void				RemoveItems() const;
	void				GiveItem( const SimpleString& ItemDef ) const;

	WBCompEldInventory*	GetInventory() const;
	EHand				GetHandEnum( WBEntity* const pItem ) const;
	WBEntity*			GetHand( const EHand Hand ) const;
	WBEntity*			GetRightHand() const;
	WBEntity*			GetLeftHand() const;
	WBEntity*			GetItemInRightHand() const;
	WBEntity*			GetItemInLeftHand() const;
	WBEntity*			GetWeapon() const;
	WBEntity*			GetWeaponAlt() const;
	WBEntity*			GetPower() const;
	WBEntity*			GetFists() const;

	int					m_HideHandsRefs;	// Serialized, refcount for hiding hands
};

#endif // WBCOMPELDHANDS_H