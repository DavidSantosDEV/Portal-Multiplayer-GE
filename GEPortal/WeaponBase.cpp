// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBase.h"
#include "GEPortalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h" 
#include "Chaos/ChaosEngineInterface.h" 
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Root"));
	RootComponent = WeaponMesh;

	WeaponMuzzle = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Muzzle"));
	WeaponMuzzle->SetupAttachment(RootComponent);

	//Replication
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->bOnlyOwnerSee = true;
	WeaponMesh->CastShadow = false;

	bReplicates = true;
}

void AWeaponBase::ClearCoolDown()
{
	bCanShoot = true;
}

void AWeaponBase::Fire()
{
	for (int i = 0; i < ShotsPerBullet;++i) {
		
		if (currentBulletCount <= 0)return;

		AActor* MyOwner = GetOwner();
		if (MyOwner) {

			//FVector StartPoint = WeaponMuzzle->GetComponentLocation();
			FRotator EyesRot;
			FVector StartPoint;
			MyOwner->GetActorEyesViewPoint(StartPoint, EyesRot);

			//Cast<AGEPortalCharacter>(MyOwner)->GetFirstPersonCameraComponent()->GetForwardVector()

			float HalfRad = FMath::DegreesToRadians(fBulletSpread);
			FVector ShotDirection = FMath::VRandCone(EyesRot.Vector(), HalfRad, HalfRad);

			FVector TraceEnd = StartPoint + (ShotDirection* fWeaponRange);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;

			QueryParams.bReturnPhysicalMaterial = true; //Thanks epic games, why does it default to falseeeee

			QueryParams.TraceTag = "DebugShot";

			#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			GetWorld()->DebugDrawTraceTag = "DebugShot";
			#endif
			

			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(Hit, StartPoint, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams)) {
				if (Hit.GetActor()) {
					if (GEngine) {
						GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("Hit: %s"), *Hit.GetActor()->GetName()));
					}
					float Dmg = FMath::FRandRange(fBaseDamage - fDamageVariation, fBaseDamage + fDamageVariation);


					//EPhysicalSurface SurfaceHit = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());// ;
					UPhysicalMaterial* PhysMatHit = Hit.PhysMaterial.Get();

					if (DamageMultiplier.Contains(PhysMatHit)) {
						Dmg *= *DamageMultiplier.Find(PhysMatHit);
					}


					UGameplayStatics::ApplyPointDamage(Hit.GetActor(), Dmg, StartPoint, Hit, GetInstigatorController(), MyOwner, WeaponDamageType);
					//Hit.Component->AddForce(TraceEnd);

					DrawDebugSphere(GetWorld(), Hit.Location, 12.f, 12, FColor::Yellow, false, 3.f);
					DrawDebugString(GetWorld(), Hit.Location, FString::Printf(TEXT("Damage: %f / Surface: %s"), Dmg, *PhysMatHit->GetName()), nullptr, FColor::White, 3.f);
				}
				
			}

			
		}
		
	}
	PlayWeaponEffects();
	if (bUseBullets) {
		currentBulletCount = FMath::Clamp(currentBulletCount - 1, 0, ClipSize);
		if (currentBulletCount <= 0)
		{
			OnFireReleased();
		}
	}


	

}

void AWeaponBase::FireSecond()
{
}

void AWeaponBase::PlayWeaponEffects_Implementation()
{
	//ParticleEmiter->Activate();
	if (WeaponMuzzle) {
		UGameplayStatics::SpawnEmitterAttached(FireEffect, WeaponMuzzle);
		//UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireEffect, WeaponMuzzle->GetComponentTransform(), true, EPSCPoolMethod::AutoRelease, true);
	}
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, WeaponMuzzle->GetComponentLocation());
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponBase, currentBulletCount);
	DOREPLIFETIME(AWeaponBase, savedAmmo);
	DOREPLIFETIME(AWeaponBase, WeaponName);
}

void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	currentBulletCount = ClipSize;
}

void AWeaponBase::ReloadWeapon()
{
	if (savedAmmo>0) {
		int neededBullets = ClipSize - currentBulletCount;
		savedAmmo -= neededBullets;
		if (savedAmmo<0) {
			neededBullets += savedAmmo;
			savedAmmo = 0;
		}
		currentBulletCount += neededBullets;
	}
}

void AWeaponBase::OnFirePressed()
{
	if (bAutomatic) {
		Fire();
		if(!CooldownHandle.IsValid())
		GetWorld()->GetTimerManager().SetTimer(CooldownHandle, this, &AWeaponBase::Fire, fWeaponCooldown,true);
	}
	else {
		if ((bCanShoot && currentBulletCount>0)||(bCanShoot && !bUseBullets))
		{
			Fire();
			bCanShoot = false;
			GetWorld()->GetTimerManager().SetTimer(CooldownHandle, this, &AWeaponBase::ClearCoolDown, fWeaponCooldown, false);
		}
	}
}

void AWeaponBase::OnFireReleased()
{
	if (bAutomatic) {
		GetWorld()->GetTimerManager().ClearTimer(CooldownHandle);
	}
}

void AWeaponBase::OnFireSecondPressed()
{
	if (bAutomatic) {
		FireSecond();
		if (!CooldownHandle.IsValid())
			GetWorld()->GetTimerManager().SetTimer(CooldownHandle, this, &AWeaponBase::Fire, fWeaponCooldown, true);
	}
	else {
		if ((bCanShoot && currentBulletCount > 0) || (bCanShoot && !bUseBullets))
		{
			FireSecond();
			bCanShoot = false;
			GetWorld()->GetTimerManager().SetTimer(CooldownHandle, this, &AWeaponBase::ClearCoolDown, fWeaponCooldown, false);
		}
	}
}

void AWeaponBase::AddBullets_Implementation(int count)
{
	if (HasAuthority()) {
		savedAmmo += count;
	}
}

void AWeaponBase::OnOwnerDeath()
{
	//Nothing
	//Just so it dont crash
}

