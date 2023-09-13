// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupBase.h"
#include "Components/CapsuleComponent.h"
#include "GEPortalCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/RotatingMovementComponent.h" 
#include "Net/UnrealNetwork.h"

// Sets default values
APickupBase::APickupBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PickupBaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupBaseMesh"));
	RootComponent = PickupBaseMesh;

	PickupRotatingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Rotating Mesh"));
	PickupRotatingMesh->SetupAttachment(RootComponent);
	PickupRotatingMesh->SetIsReplicated(true);
	PickupRotatingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Pickup Collision"));
	PickupCollision->SetupAttachment(RootComponent);

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotating Movement"));
	RotatingMovement->SetUpdatedComponent(PickupRotatingMesh);
	bReplicates = true;
}

void APickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APickupBase, bIsActive);
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &APickupBase::OnBeginOverlap);
}

void APickupBase::ServerResetPickup_Implementation()
{
	if (HasAuthority()) {
		OnReset();
	}
}

void APickupBase::OnReset()
{
	bIsActive = true;
	PickupRotatingMesh->SetVisibility(true);
}

void APickupBase::OnRep_IsActive()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("Pickup %s Active State %s"), *this->GetName(), bIsActive ? TEXT("Active") : TEXT("Deactivated")));
	}
	PickupRotatingMesh->SetHiddenInGame(!bIsActive);
}

void APickupBase::ServerPickedUp_Implementation(AGEPortalCharacter* Player)
{
	if (HasAuthority()) 
	{
		if (bIsActive) {
			if (MeetsPickupConditions(Player)) {
				OnPickedUp(Player);
			}		
		}
	}
}

void APickupBase::OnPickedUp(AGEPortalCharacter* Player)
{
	if (bAutoRespawn) {
		GetWorld()->GetTimerManager().SetTimer(PickupTimerHandle, this, &APickupBase::ServerResetPickup, fPickupCooldown, false);
	}
	bIsActive = false;
	PickupRotatingMesh->SetVisibility(false);
}

bool APickupBase::MeetsPickupConditions(AGEPortalCharacter* Player)
{
	return false;
}

void APickupBase::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bIsActive) {
		AGEPortalCharacter* Character = Cast<AGEPortalCharacter>(OtherActor);
		if (Character) {
			ServerPickedUp(Character);
		}
	}

}

