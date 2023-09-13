// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GEPortalCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UPortalManagerComponent;
class AWeaponBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponChangedDel, AWeaponBase*, NewWeapon);


UCLASS(config=Game)
class AGEPortalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	bool bIsLookingAtPortalWall=false;



protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	class UHealthComponent* HealthComp;

	UPROPERTY(Replicated,EditDefaultsOnly, BlueprintReadOnly, Category = Weapons)
	TArray<TSubclassOf<AWeaponBase>> DefaultWeapons;

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing= OnRep_WeaponIndex)
	int WeaponIndex = 0;

	UPROPERTY(BlueprintReadOnly, Replicated)
	TArray<class AWeaponBase*> WeaponList;
public:
	AGEPortalCharacter();

	UFUNCTION()
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FOnWeaponChangedDel OnWeaponChanged;

protected:

	UFUNCTION()
	void OnRep_WeaponIndex();

	class ACustomPlayerController* CastedController;

	void UpdateWeaponUI(AWeaponBase* Weapon);

	UFUNCTION()
	void RemoveAllWeapons();

	virtual void BeginPlay();

	void NextWeapon();
	void PreviousWeapon();

	UFUNCTION(Server, Reliable)
	void ChangeWeapon(int valueChange);
	void ChangeWeapon_Implementation(int valueChange);

	void SetWeaponActive(AWeaponBase* Weapon, AWeaponBase* PreviousWeapon);

	//UFUNCTION(Server, Reliable)
	void CreateDefaultWeapons();
	//Doesnt need to be server func

	void FireWeaponPrimary();
	void ReleaseWeaponPrimary();

	void FireWeaponSecondary();

	UFUNCTION(Server, Reliable)
	void ReloadWeapon();
	void ReloadWeapon_Implementation();

	UFUNCTION(Server, Unreliable)
	void ServerFireWeaponPrimary();
	void ServerFireWeaponPrimary_Implementation();
	UFUNCTION(Server, Unreliable)
	void ServerReleaseWeaponPrimary();
	void ServerReleaseWeaponPrimary_Implementation();

	UFUNCTION(Server, Unreliable)
	void ServerFireWeaponSecondary();
	void ServerFireWeaponSecondary_Implementation();

	UFUNCTION(BlueprintImplementableEvent)
	void SetReplicatedWeaponColor(FColor WeaponColor);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	UFUNCTION()
	void CallOnDead(AController* MyKiller);

	
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

public:

	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable, Category = "Character")
	void ChangePlayerColor(FLinearColor Color);

	UFUNCTION(NetMulticast, Reliable)
	void OnDieEvent(AController* MyKiller);
	void OnDieEvent_Implementation(AController* MyKiller);

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void GiveWeapon(TSubclassOf<AWeaponBase> WeaponClass, bool bAutoEquip);

	bool HasWeapon(TSubclassOf<AWeaponBase> WeaponInQuestion);

	AWeaponBase* GetWeaponOfClass(TSubclassOf<AWeaponBase> WeaponClass);

	UFUNCTION(BlueprintPure, BlueprintCallable)
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	UFUNCTION(BlueprintPure, BlueprintCallable)
	UHealthComponent* const GetHealthComponent() { return HealthComp; };

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Portals")
	bool GetIsLookingAtPortalWall();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player Weapon")
	AWeaponBase* GetPlayerCurrentWeapon();
};

