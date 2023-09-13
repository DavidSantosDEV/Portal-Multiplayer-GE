// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PortalManagerComponent.generated.h"

class APortalActor;
class UTextureRenderTarget2D;
class AGEPortalProjectile;


//Deprecated
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GEPORTAL_API UPortalManagerComponent : public UActorComponent
{
	GENERATED_BODY()
	
	APortalActor* PortalA;
	APortalActor* PortalB;

	FCollisionObjectQueryParams ObjectParams;
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TSubclassOf<APortalActor> PortalClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portals")
	TSubclassOf<AGEPortalProjectile> ProjectileClass;

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

public:
	// Sets default values for this component's properties
	UPortalManagerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void UpdatePortals();

	bool CheckPortalPlacement(FHitResult OriginalHit, FVector& OutPosition);

	float GetPortalRadius();
	float GetPortalHeight();

	void SetupPortalAs(APortalActor* Portal, bool bBlue);
	void LinkPortals(APortalActor* FirstPortal, APortalActor* SecondPortal);

	UTextureRenderTarget2D* GeneratePortalTexture();

	void ClearPortal(bool bPortalA);
public:

	UFUNCTION(BlueprintCallable, Category = "Portals")
	void PlacePortal(FVector StartPoint, FVector DirectionVector, bool bBlue);
	UFUNCTION(BlueprintCallable, Category = "Portals")
	void FireProjectile(FVector SpawnLocation, FRotator SpawnRotation, bool bBlue);

	void PlacePortalAt(FHitResult OriginalHit, bool bBlue);


	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Portals")
	bool IsFacingPortalWall(FVector StartPoint, FVector DirectionVector);
		
	void ClearAllPortals();

};
