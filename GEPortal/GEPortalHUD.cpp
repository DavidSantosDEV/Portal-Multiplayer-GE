// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalHUD.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "TextureResource.h"
#include "CanvasItem.h"
#include "GEPortalCharacter.h"
#include "PortalManagerComponent.h"
#include "UObject/ConstructorHelpers.h"

AGEPortalHUD::AGEPortalHUD()
{
}

void AGEPortalHUD::BeginPlay()
{
	OwningPlayer = Cast<AGEPortalCharacter>(GetOwningPawn());
}
