// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PortalGun.h"
#include "Components/SphereComponent.h"

void AGEPortalProjectile::SetOwningWeapon(APortalGun* Owning)
{
	OwningGun = Owning;
}

void AGEPortalProjectile::SetPortalType(bool bBlue)
{
	IsBluePortal = bBlue;
}

void AGEPortalProjectile::ReactOnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if ((OtherActor != nullptr))
	{
		if (OwningGun) {
			OwningGun->PlacePortalAt(Hit, IsBluePortal);
		}

		Destroy();
	}
}