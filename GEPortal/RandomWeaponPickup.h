// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "RandomWeaponPickup.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API ARandomWeaponPickup : public APickupBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponPickup")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Pickup")
	TSubclassOf<class AWeaponBase> WeaponsToPick;
protected:
	virtual void OnPickedUp(class AGEPortalCharacter* Player) override;

	virtual bool MeetsPickupConditions(AGEPortalCharacter* Player) override;

	virtual void OnRep_IsActive() override;

	virtual void OnReset() override;
public:
	ARandomWeaponPickup();
};
