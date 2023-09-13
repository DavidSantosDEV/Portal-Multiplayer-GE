// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

UCLASS(Abstract)
class GEPORTAL_API APickupBase : public AActor
{
	GENERATED_BODY()
protected:

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = "Pickup")
	class UCapsuleComponent* PickupCollision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,Category = "Pickup")
	class UStaticMeshComponent* PickupBaseMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class UStaticMeshComponent* PickupRotatingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	class URotatingMovementComponent* RotatingMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	float fPickupCooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup")
	bool bAutoRespawn=true;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive)
	bool bIsActive = true;

	FTimerHandle PickupTimerHandle;

public:	
	// Sets default values for this actor's properties
	APickupBase();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerResetPickup();
	void ServerResetPickup_Implementation();

	virtual void OnReset();

	UFUNCTION()
	virtual void OnRep_IsActive();

	UFUNCTION(Server, Reliable)
	void ServerPickedUp(class AGEPortalCharacter* Player);
	void ServerPickedUp_Implementation(AGEPortalCharacter* Player);

	virtual void OnPickedUp(AGEPortalCharacter* Player);

	virtual bool MeetsPickupConditions(AGEPortalCharacter* Player);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
