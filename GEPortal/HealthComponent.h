// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDie, AController*, MyKiller);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GEPORTAL_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HealthComponent")
	float fMaxHealth = 100.f;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "HealthComponent")
	float fCurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "HealthComponent")
	bool bIsAlive=true;

	UFUNCTION()
	void HandleOnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

public:

	UFUNCTION(BlueprintCallable, Category = "HealthComponent")
	void Heal(float fAmmount);

	UFUNCTION(BlueprintPure,BlueprintCallable, Category = "HealthComponent")
	FORCEINLINE	bool GetIsAlive() { return bIsAlive; };

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, VisibleAnywhere, Category = "HealthComponent")
	FOnDie OnDeathEvent;
};
