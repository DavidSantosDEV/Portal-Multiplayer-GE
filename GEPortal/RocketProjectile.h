// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileBase.h"
#include "RocketProjectile.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API ARocketProjectile : public AProjectileBase
{
	GENERATED_BODY()
protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	class USoundBase* ExplosionSound;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	class UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rocket")
	float DamageToApply=30.f;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Rocket")
	float RadiousOfExplosion= 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rocket")
	TSubclassOf<UDamageType> DamageType;

protected:
	virtual void ReactOnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;

	UFUNCTION(NetMulticast, Reliable)
	void PlayEffects(FVector Location);
	void PlayEffects_Implementation(FVector Location);
};
