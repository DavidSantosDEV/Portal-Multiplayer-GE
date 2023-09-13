// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"


// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwnerRole()==ROLE_Authority) {
		GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleOnTakeAnyDamage);
		//Ensure this event is only applied to server.
	}
	fCurrentHealth = fMaxHealth;
}

void UHealthComponent::HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsAlive) {
		fCurrentHealth = FMath::Clamp(fCurrentHealth - Damage, 0.f, fMaxHealth);
		if (fCurrentHealth == 0.f) {
			bIsAlive = false;
			OnDeathEvent.Broadcast(InstigatedBy);
		}
	}
}

void UHealthComponent::Heal(float fAmmount)
{
	if (bIsAlive && fCurrentHealth != fMaxHealth) {
		fCurrentHealth = FMath::Clamp(fCurrentHealth + fAmmount, 0.f, fMaxHealth);
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, fCurrentHealth);
}
