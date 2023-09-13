// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CustomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void CreateWeaponUI();

	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponUI(class AWeaponBase* Weapon);
};
