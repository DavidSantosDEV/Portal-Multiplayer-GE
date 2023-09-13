// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PortalWall.generated.h"

UCLASS()
class GEPORTAL_API APortalWall : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PortalWallMesh;
protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Portal Wall")
	float fWallWidth = 10.f;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Portal Wall")
	float fWallHeight = 10.f;
public:	
	// Sets default values for this actor's properties
	APortalWall();
};
