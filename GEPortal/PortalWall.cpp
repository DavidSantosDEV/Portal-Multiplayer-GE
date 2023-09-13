// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalWall.h"

// Sets default values
APortalWall::APortalWall()
{
	PrimaryActorTick.bCanEverTick = false;
	PortalWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wall Mesh"));
	RootComponent = PortalWallMesh;
}

