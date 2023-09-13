// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomWeaponPickup.h"
#include "GEPortalCharacter.h"
#include "Components/SkeletalMeshComponent.h" 
#include "GameFramework/RotatingMovementComponent.h" 

void ARandomWeaponPickup::OnPickedUp(AGEPortalCharacter* Player)
{
	Super::OnPickedUp(Player);
	if (Player) {
		WeaponMesh->SetVisibility(false);
		Player->GiveWeapon(WeaponsToPick, true);
	}
}

bool ARandomWeaponPickup::MeetsPickupConditions(AGEPortalCharacter* Player)
{
	return !Player->HasWeapon(WeaponsToPick);
}

void ARandomWeaponPickup::OnRep_IsActive()
{
	Super::OnRep_IsActive();
	WeaponMesh->SetVisibility(bIsActive);
}

void ARandomWeaponPickup::OnReset()
{
	Super::OnReset();
	WeaponMesh->SetVisibility(true);
}

ARandomWeaponPickup::ARandomWeaponPickup() : Super()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);

	PickupRotatingMesh->SetVisibility(false);

	RotatingMovement->SetUpdatedComponent(WeaponMesh);
}
