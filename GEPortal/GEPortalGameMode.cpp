// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalGameMode.h"
#include "GEPortalHUD.h"
#include "GEPortalCharacter.h"
#include "CustomPlayerState.h"
#include "GameFramework/PlayerState.h" 
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

void AGEPortalGameMode::RespawnPlayer_Implementation(AController* PController)
{
	if (!PController) return;
	if (!PController->GetPawn()) return;

	APawn* Pawn = PController->GetPawn();
	Pawn->Reset();
	Pawn->Destroy();
	if (MyAbsoluteState == StateOfGame::Start) {
		RestartPlayer(PController);
	}
	else {
		RestartPlayerAtTransform(PController, GetRandomSpawnTransform());
	}
	
	PController->GetPawn();
	AGEPortalCharacter* Char = Cast<AGEPortalCharacter>(PController->GetPawn());
	if (Char) {
		APlayerState* PlayerState;
		PlayerState = PController->GetPlayerState<APlayerState>();
		if (PlayerState) {
			Char->ChangePlayerColor(GetColorForPlayer(PlayerState));
		}
	}
	
}

void AGEPortalGameMode::PrepareController_Implementation(APawn* Pawn, AController* Controller)
{
	Controller->Possess(Pawn);
	Pawn->EnableInput(Cast<APlayerController>(Controller));
	// Set initial control rotation to starting rotation rotation
	Controller->ClientSetRotation(Controller->GetPawn()->GetActorRotation(), true);

	SetPlayerDefaults(Controller->GetPawn());
}

APawn* AGEPortalGameMode::CreatePawnOf(TSubclassOf<APawn> PawnClass, FTransform PawnTransform)
{
	if (PawnClass) {
		FActorSpawnParameters SpawnParams;
		return GetWorld()->SpawnActor<APawn>(PawnClass, PawnTransform, SpawnParams);
		
	}
	return nullptr;
}

FTransform AGEPortalGameMode::GetRandomSpawnTransform()
{
	if (RandomSpawnPoints.Num()>0) {
		return RandomSpawnPoints[FMath::RandRange(0, RandomSpawnPoints.Num() - 1)];
	}
	return FTransform();
}

void AGEPortalGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//AGameStateBase* GameState = GetGameState<AGameStateBase>();
	if (GameState) {
		int numPlayers = GameState->PlayerArray.Num();
		if (numPlayers>4) {
			UGameplayStatics::RemovePlayer(NewPlayer, true); //Why ya trynna join man
		}
		if (numPlayers==4) {			
			//Start countdown
			GetWorld()->GetTimerManager().SetTimer(HandleBegin, this, &AGEPortalGameMode::StartGame, fBeginGameTime, false);
		}
	}
}

void AGEPortalGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	//Stop timer if one guy leaves mid countdown
	switch (MyAbsoluteState)
	{
	case Start:
		if (GameState) {
			if (GameState->PlayerArray.Num() < 4) {
				if (HandleBegin.IsValid()) {
					GetWorld()->GetTimerManager().ClearTimer(HandleBegin);
				}
			}
		}
		break;
	case Midway:
		break;
	case Ending:
		break;
	default:
		break;
	}
}

void AGEPortalGameMode::StartGame()
{
	//AGameStateBase* GameState = GetGameState<AGameStateBase>();
	if (GameState) {
	 	MyAbsoluteState = StateOfGame::Midway;
		for (int i = 0; i < GameState->PlayerArray.Num(); ++i)
		{
			APawn* PlayerPawn = GameState->PlayerArray[i]->GetPawn();
			if (PlayerPawn && PlayerPawn->GetController()) {
				RespawnPlayer(PlayerPawn->GetController());
			}
		}
	}

	FTimerHandle GameTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(GameTimerHandle, this, &AGEPortalGameMode::EndGame, GameMinutes * 60, false);
	//Do Other things
}

void AGEPortalGameMode::EndGame()
{
}

AGEPortalGameMode::AGEPortalGameMode()
	: Super()
{
	HUDClass = AGEPortalHUD::StaticClass();
}

void AGEPortalGameMode::OnPlayerEliminated(AGEPortalCharacter* Player, AController* VictimController, AController* Killer)
{
	if (!Player || !Killer) {
		return;
	}
	if (HasAuthority()) {
		FDetachmentTransformRules RulesOfNature = FDetachmentTransformRules(EDetachmentRule::KeepRelative,false);
		//VictimController->DetachFromActor(RulesOfNature);
		Player->OnDieEvent(Killer);
		if (MyAbsoluteState == Midway) {
			APlayerController* KillerCP = Cast<APlayerController>(Killer);
			ACustomPlayerState* KillerState = (ACustomPlayerState*)Killer->GetPawn()->GetPlayerState();
			ACustomPlayerState* VictimState = (ACustomPlayerState*)Player->GetPlayerState();
			if (KillerState) {
				KillerState->AddScore(fKillValue);
				KillerState->AddKill();
			}
			if (VictimState) {
				
				VictimState->AddScore(-fKillValue / 2);
				VictimState->AddDeath();
			}
		}
		//Set Timer
		FTimerDelegate TimerDel;
		FTimerHandle TimerHandle;

		TimerDel.BindUFunction(this, FName("RespawnPlayer"), VictimController);

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 5.f, false);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, FString::Printf(TEXT("Player %s died"), *Player->GetName()));
		}
	}
}

void AGEPortalGameMode::AddSpawnTransform(FTransform newTransform)
{
	RandomSpawnPoints.Add(newTransform);
}

bool AGEPortalGameMode::CanDamageBeApplied()
{
	return false;
}
