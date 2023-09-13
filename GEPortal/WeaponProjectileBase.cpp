// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponProjectileBase.h"
#include "GEPortalCharacter.h"

void AWeaponProjectileBase::Fire()
{
	UWorld* const World = GetWorld();
	if (World) {
		if (ProjectileClass) {

			const FVector SpawnPoint = WeaponMuzzle->GetComponentLocation();

			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Owner = this;
			APawn* OwningPawn = Cast<APawn>(GetOwner());
			if (OwningPawn) {
				ActorSpawnParams.Instigator = OwningPawn;
				FRotator ControlRotation = OwningPawn->GetControlRotation();
				// spawn the projectile at the muzzle
				World->SpawnActor<AActor>(ProjectileClass, SpawnPoint, ControlRotation, ActorSpawnParams);
			}
		}
	}
	PlayWeaponEffects();
	currentBulletCount = FMath::Clamp(currentBulletCount - 1, 0, ClipSize);
	if (currentBulletCount <= 0)
	{
		OnFireReleased();
	}
	
}
