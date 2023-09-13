// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GEPortalHUD.generated.h"

UCLASS()
class AGEPortalHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGEPortalHUD();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	class AGEPortalCharacter* OwningPlayer;

};

