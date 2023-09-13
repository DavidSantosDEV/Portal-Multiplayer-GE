// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CustomPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GEPORTAL_API ACustomPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(Replicated,BlueprintReadOnly, Category = "PlayerState")
	int KillCount;
	UPROPERTY(Replicated,BlueprintReadOnly, Category = "PlayerState")
	int DeathCount;

public:
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddScore(float ScoreAdd);

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddKill();
	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	void AddDeath();

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "PlayerState")
	int GetKills();
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "PlayerState")
	int GetDeaths();


	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
};
