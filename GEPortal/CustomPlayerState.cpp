// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerState.h"
#include "Net/UnrealNetwork.h"

void ACustomPlayerState::AddScore(float ScoreAdd)
{
	SetScore(GetScore() + ScoreAdd);
	//I think negative scores are funnier than having a clamped value at 0
}

void ACustomPlayerState::AddKill()
{
	KillCount++;
}

void ACustomPlayerState::AddDeath()
{
	DeathCount++;
}

int ACustomPlayerState::GetKills()
{
	return KillCount;
}

int ACustomPlayerState::GetDeaths()
{
	return DeathCount;
}

void ACustomPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACustomPlayerState, KillCount);
	DOREPLIFETIME(ACustomPlayerState, DeathCount);
}
