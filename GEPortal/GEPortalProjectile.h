// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.h"
#include "GEPortalProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AGEPortalProjectile : public AProjectileBase
{
	GENERATED_BODY()

	bool IsBluePortal;
	class APortalGun* OwningGun;
protected:

	

public:
	void SetOwningWeapon(APortalGun* Owning);

	void SetPortalType(bool bBlue);

	UFUNCTION(BlueprintImplementableEvent)
	void SetProjectileColor(FLinearColor newColor);

	/** called when projectile hits something */

	virtual void ReactOnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
};

