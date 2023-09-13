// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chaos/ChaosEngineInterface.h" 
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "WeaponBase.generated.h"

UCLASS(Abstract)
class GEPORTAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()



public:
	// Sets default values for this actor's properties
	AWeaponBase();

protected:

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly, Category = Mesh)
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleDefaultsOnly,BlueprintReadOnly, Category = "Weapon")
	USceneComponent* WeaponMuzzle;
	
	UPROPERTY(Replicated)
	int currentBulletCount = 0;

	FTimerHandle CooldownHandle;

	UPROPERTY(BlueprintReadOnly)
	bool bCanShoot = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", Replicated)
	FName WeaponName = "Gun";
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Weapon")
	class USoundBase* FireSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	class UParticleSystem* FireEffect;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FColor WeaponColor = FColor::White;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UMaterialInterface* MatInterface;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int ClipSize = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bAutomatic;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	bool bUseBullets = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float fWeaponCooldown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float fWeaponRange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int ShotsPerBullet = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float fBulletSpread=0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float fBaseDamage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float fDamageVariation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TMap<UPhysicalMaterial*, float> DamageMultiplier;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<UDamageType> WeaponDamageType;

	UPROPERTY(Replicated)
	int savedAmmo;
protected:

	void ClearCoolDown();

	virtual void Fire();
	virtual void FireSecond();

	UFUNCTION(NetMulticast, Unreliable)
	void PlayWeaponEffects();
	void PlayWeaponEffects_Implementation();
public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	virtual void BeginPlay() override;

	void ReloadWeapon();

	virtual void OnFirePressed();
	virtual void OnFireReleased();

	virtual void OnFireSecondPressed();

	UFUNCTION(Server, Reliable)
	void AddBullets(int count);
	void AddBullets_Implementation(int count);


	virtual void OnOwnerDeath();


	FORCEINLINE const USkeletalMeshComponent* GetWeaponMesh() { return WeaponMesh; };

	FORCEINLINE const FColor GetWeaponColor() { return WeaponColor; };

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapon")
	int GetCurrentBulletCount() { return currentBulletCount; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapon")
	int GetStoredBulletCount() { return savedAmmo; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapon")
	int GetClipMaxAmmo() { return ClipSize; };
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Weapon")
	FORCEINLINE FName GetWeaponName() { return WeaponName; };

};
