// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "ProjectileBase.h"
#include "WeaponProjectileBase.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API AWeaponProjectileBase : public AWeaponBase
{
	GENERATED_BODY()
protected:

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Projectile Weapon")
	TSubclassOf<class AProjectileBase> ProjectileClass;

protected:
	virtual void Fire() override;
};
