// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "BulletPickup.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API ABulletPickup : public APickupBase
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BulletPickup")
	int bulletCount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BulletPickup")
	TSubclassOf<class AWeaponBase> DesignatedWeapon;

protected:
	virtual void OnPickedUp(AGEPortalCharacter* Player) override;

	virtual bool MeetsPickupConditions(AGEPortalCharacter* Player) override;
};
