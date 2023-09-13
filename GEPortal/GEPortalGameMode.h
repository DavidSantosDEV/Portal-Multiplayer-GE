// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GEPortalGameMode.generated.h"

UENUM(BlueprintType)
enum StateOfGame {
	Start,
	Midway,
	Ending,
};


UCLASS(minimalapi)
class AGEPortalGameMode : public AGameModeBase
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category = "Gamemode")
	float fKillValue = 10;

	UPROPERTY(BlueprintReadWrite, Category = "Gamemode")
	TArray<FTransform> RandomSpawnPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	int GameMinutes = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gamemode")
	float fBeginGameTime = 5.f;

	//THE ABSOLUTE STATE OF PLAY
	StateOfGame MyAbsoluteState;

	FTimerHandle HandleBegin;

protected:

	UFUNCTION(Server, Reliable, Category = "Gamemode Multiplayer")
	void RespawnPlayer(class AController* PController);
	void RespawnPlayer_Implementation(AController* PController);

	UFUNCTION(NetMulticast, Reliable)
	void PrepareController(APawn* Pawn, AController* Controller);
	void PrepareController_Implementation(APawn* Pawn, AController* Controller);

	APawn* CreatePawnOf(TSubclassOf<APawn> PawnClass, FTransform PawnTransform);

	FTransform GetRandomSpawnTransform();

	UFUNCTION(BlueprintImplementableEvent, Category = "Gamemode")
	FLinearColor GetColorForPlayer(APlayerState* State);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void StartGame();
	void EndGame();

public:
	AGEPortalGameMode();

	void OnPlayerEliminated(class AGEPortalCharacter* Player, AController* VictimController,AController* Killer);

	UFUNCTION(BlueprintCallable, Category = "Gamemode Multiplayer")
	void AddSpawnTransform(FTransform newTransform);

	UFUNCTION(BlueprintPure, Category = "Gamemode Multiplayer")
	bool CanDamageBeApplied();
};



