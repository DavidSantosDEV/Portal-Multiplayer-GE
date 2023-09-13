// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletPickup.h"
#include "GEPortalCharacter.h"
#include "WeaponBase.h"

void ABulletPickup::OnPickedUp(AGEPortalCharacter* Player)
{
	Super::OnPickedUp(Player);
	AWeaponBase* Weapon=Player->GetWeaponOfClass(DesignatedWeapon);
	if (Weapon) {
		Weapon->AddBullets(bulletCount);
	}
}

bool ABulletPickup::MeetsPickupConditions(AGEPortalCharacter* Player)
{
	if (!DesignatedWeapon) {
		return false;
	}
	return Player->HasWeapon(DesignatedWeapon);
}
