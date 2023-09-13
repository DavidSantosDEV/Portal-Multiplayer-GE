// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UCapsuleComponent;
class AGEPortalCharacter;
UCLASS()
class GEPORTAL_API APortalActor : public AActor
{
	GENERATED_BODY()
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	USceneComponent* PortalRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	USceneComponent* PortalCheat;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	UCapsuleComponent* CapsuleCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	UStaticMeshComponent* PortalBorderMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	UStaticMeshComponent* PortalBackground;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Portals)
	USceneCaptureComponent2D* SceneCaptureCamera;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = Portals)
	float NearClipOffset = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Portals)
	float PortalPushForce = 50.f;

	UPROPERTY(BlueprintReadOnly, Category = Portals)
	TArray<AGEPortalCharacter*> PlayersInPortal;

	UPROPERTY(Replicated)
	APortalActor* Target;

	UTextureRenderTarget2D* MyRenderTexture;
public:	
	// Sets default values for this actor's properties
	APortalActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	void CheckShouldTeleport(AGEPortalCharacter* ActorToTeleport);

	void TeleportActor(AGEPortalCharacter* ActorToTeleport);

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	USceneCaptureComponent2D* GetCaptureCamera() const { return SceneCaptureCamera; }
	UTextureRenderTarget2D* GetRenderTexture();

	UFUNCTION(Server, Reliable,BlueprintCallable, Category = Portals)
	void SetPortalTarget(APortalActor* newTarget);
	void SetPortalTarget_Implementation(APortalActor* newTarget);

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = Portal)
	APortalActor* GetTarget() { return Target; }

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = Portals)
	void SetRenderTexture(UTextureRenderTarget2D* RenderTexture);
	void SetRenderTexture_Implementation(UTextureRenderTarget2D* RenderTexture);


	UFUNCTION(BlueprintImplementableEvent,BlueprintCallable, Category = Portals)
	void SetViewTexture(UTextureRenderTarget2D* ViewTexture);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = Portals)
	void SetPortalColor(FLinearColor Color);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
