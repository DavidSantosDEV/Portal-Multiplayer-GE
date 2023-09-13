// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponProjectileBase.h"
#include "PortalGun.generated.h"

/**
 * 
 */

class APortalActor;
class UTextureRenderTarget2D;
class AGEPortalProjectile;
UCLASS()
class GEPORTAL_API APortalGun : public AWeaponProjectileBase
{
	GENERATED_BODY()

	APortalActor* PortalA;
	APortalActor* PortalB;

	FCollisionObjectQueryParams ObjectParams;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TSubclassOf<AActor> SpawnAfterDeath;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TSubclassOf<APortalActor> PortalClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	float fPortalTraceDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	float correctRadiusPortal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	float correctHeightPortal;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	float TraceLocationDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TArray<TEnumAsByte<ECollisionChannel>>ObjectsToQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TEnumAsByte<ECollisionChannel> PortalCollisionChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TEnumAsByte<ETraceTypeQuery> PortalQueryChannel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	UTextureRenderTarget2D* TextureBlue;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	UTextureRenderTarget2D* TextureOrange;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	FLinearColor BlueColor = FLinearColor::Blue;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	FLinearColor OrangeColor = FLinearColor::Red;

	APlayerController* OwnerController;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void Fire() override;
	virtual void FireSecond() override;

	bool CheckPortalPlacement(FHitResult OriginalHit, FVector& OutPosition);

	float GetPortalRadius();
	float GetPortalHeight();

	void SetupPortalAs(APortalActor* Portal, bool bBlue);

	void LinkPortals(APortalActor* FirstPortal, APortalActor* SecondPortal);

	UTextureRenderTarget2D* GeneratePortalTexture();

	void ClearPortal(bool bPortalA);

	UFUNCTION(Server, Reliable)
	void ServerSpawnPickup();
	void ServerSpawnPickup_Implementation();

public:

	UFUNCTION(BlueprintCallable, Category = "Portals")
	void FireProjectile(bool bBlue);

	UFUNCTION(Server, Reliable)
	void PlacePortalAt(FHitResult OriginalHit, bool bBlue);
	void PlacePortalAt_Implementation(FHitResult OriginalHit, bool bBlue);

	//virtual void OnOwnerDeath() override;

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Portals")
	bool IsFacingPortalWall(FVector StartPoint, FVector DirectionVector);

	void ClearAllPortals();
};
