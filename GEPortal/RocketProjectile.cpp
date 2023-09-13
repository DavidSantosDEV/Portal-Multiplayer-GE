// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

//If this was to be done correctly a base class for damaging projectiles would be done, and then a subclass for this would exist, this is just a simple project so its like this

void ARocketProjectile::ReactOnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	TArray<AActor*> IgnoreThese;
	IgnoreThese.AddUnique(GetInstigator());
	IgnoreThese.AddUnique(this);

	UGameplayStatics::ApplyRadialDamage(GetWorld(), DamageToApply, Hit.Location, RadiousOfExplosion, DamageType, IgnoreThese, GetInstigator(), GetInstigatorController(), false);
	DrawDebugSphere(GetWorld(), Hit.Location, RadiousOfExplosion, 12, FColor::Red, false, 3.f);

	PlayEffects(Hit.Location);
	Destroy();
}

void ARocketProjectile::PlayEffects_Implementation(FVector Location)
{
	if (ExplosionEffect) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, Location);
	}
	if (ExplosionSound) {
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ExplosionSound, Location);
	}
}
