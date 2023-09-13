// Fill out your copyright notice in the Description page of Project Settings.

#include "PortalManagerComponent.h"
#include "DrawDebugHelpers.h" 
#include "Kismet/GameplayStatics.h"
#include "GEPortalCharacter.h"
#include "PortalWall.h"
#include "PortalActor.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "EngineUtils.h"
#include "GEPortalProjectile.h"

// Sets default values for this component's properties
UPortalManagerComponent::UPortalManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	correctRadiusPortal = 12.f;
	correctHeightPortal = 20.f;
	TraceLocationDistance = 100.f;
	// ...
}


// Called when the game starts
void UPortalManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	for (int i = 0; i < ObjectsToQuery.Num(); ++i) ObjectParams.AddObjectTypesToQuery(ObjectsToQuery[i]);
	// ...
	OwnerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());
}

void UPortalManagerComponent::UpdatePortals()
{
	if (OwnerController) {
		APlayerCameraManager* CameraManager = OwnerController->PlayerCameraManager;
		if(CameraManager) {
			for (TActorIterator<APortalActor> ActorIt(GetWorld()); ActorIt; ++ActorIt) {
				APortalActor* Portal = *ActorIt;
				APortalActor* PortalTarget = Portal->GetTarget();
				if (Portal && PortalTarget) {

				}
			}
		}
	}
}


void UPortalManagerComponent::PlacePortal(FVector StartPoint, FVector DirectionVector, bool bBlue)
{
	if (!PortalClass) {
		return;
	}
	UWorld* const World = GetWorld();
	if (World) {
		FVector Start = StartPoint;
		FVector End = Start + DirectionVector * fPortalTraceDistance;
		FHitResult outHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		if(World->LineTraceSingleByChannel(outHit, Start, End, ECollisionChannel::ECC_Visibility, QueryParams)) {
			ECollisionChannel channel = outHit.Component->GetCollisionObjectType();
			UE_LOG(LogTemp, Warning, TEXT("Surface Hit %s"),*UEnum::GetValueAsString(channel));
			if (channel == PortalCollisionChannel) {
				FVector portalPosition;
				if (CheckPortalPlacement(outHit,portalPosition)) {
					//Place portal
					UE_LOG(LogTemp, Warning, TEXT("Yay"));
					FActorSpawnParameters SpawnParams;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
					APortalActor* Portal = World->SpawnActor<APortalActor>(PortalClass, outHit.Location, outHit.Actor->GetActorRotation(),SpawnParams);
					if (Portal)
					{
						ClearPortal(bBlue);
						SetupPortalAs(Portal, bBlue);
					}
					//World->SpawnActor<APickup>(WhatToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("Nay"));
				}
			}
		}
	}
}

void UPortalManagerComponent::FireProjectile(FVector SpawnLocation, FRotator SpawnRotation, bool bBlue)
{
	if (ProjectileClass != nullptr)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			
			AGEPortalProjectile* Projectile = World->SpawnActor<AGEPortalProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			if (Projectile) {
				//Projectile->SetOwningComponent(this);
				Projectile->SetPortalType(bBlue);
				Projectile->SetProjectileColor(bBlue ? BlueColor : OrangeColor);
			}
		}
	}
}

void UPortalManagerComponent::PlacePortalAt(FHitResult OriginalHit, bool bBlue)
{
	UWorld* const World = GetWorld();
	if (!World) return;
	ECollisionChannel channel = OriginalHit.Component->GetCollisionObjectType();
	UE_LOG(LogTemp, Warning, TEXT("Surface Hit %s"), *UEnum::GetValueAsString(channel));
	if (channel == PortalCollisionChannel) {
		FVector portalPosition;
		if (CheckPortalPlacement(OriginalHit, portalPosition)) {
			//Place portal
			UE_LOG(LogTemp, Warning, TEXT("Yay"));

			FActorSpawnParameters SpawnParams;

			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			APortalActor* Portal = World->SpawnActor<APortalActor>(PortalClass, OriginalHit.Location, OriginalHit.Actor->GetActorRotation(), SpawnParams);
			if (Portal)
			{
				ClearPortal(bBlue);
				SetupPortalAs(Portal, bBlue);
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Nay"));
		}
	}
}

bool UPortalManagerComponent::IsFacingPortalWall(FVector StartPoint, FVector DirectionVector)
{
	FVector Start = StartPoint;
	FVector End = Start + DirectionVector * fPortalTraceDistance;
	FHitResult outHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	const UWorld* World = GetWorld();
	if (!World) {
		return false;
	}
	if (World->LineTraceSingleByChannel(outHit, Start, End, ECollisionChannel::ECC_Visibility, QueryParams)) {
		ECollisionChannel channel = outHit.Component->GetCollisionObjectType();
		return channel == PortalCollisionChannel;
	}
	else {
		return false;
	}
}

void UPortalManagerComponent::ClearAllPortals()
{
	if(PortalA)
	PortalA->Destroy();
	if(PortalB)
	PortalB->Destroy();
	PortalA = nullptr;
	PortalB = nullptr;
}

void UPortalManagerComponent::ClearPortal(bool bPortalA)
{
	APortalActor* PortalToClear = bPortalA ? PortalA : PortalB;
	if (PortalToClear) {
		PortalToClear->Destroy();
	}
}

bool UPortalManagerComponent::CheckPortalPlacement(FHitResult OriginalHit, FVector& OutPosition)
{
	AActor* CurrentWall = OriginalHit.GetActor();
	FVector PortalPosition = OriginalHit.Location;

	FVector LocalHitLocation = UKismetMathLibrary::InverseTransformLocation(CurrentWall->GetActorTransform(), OriginalHit.Location);

	FVector InitialTracePoint = OriginalHit.Normal * TraceLocationDistance + PortalPosition;
	//DrawDebugSphere(GetWorld(), InitialTracePoint, 32.f, 12, FColor::Yellow, false, 10.f);

	UWorld* const World = GetWorld();
	if (World)
	{
		//const FName TraceTag("MyTraceTag");
		//World->DebugDrawTraceTag = TraceTag;

		FCollisionQueryParams Params;
		//Params.TraceTag = TraceTag;


		TArray<AActor*> IgnoreBox;
		IgnoreBox.Add(OriginalHit.GetActor());
		FHitResult outHitBox;
		bool bHit = UKismetSystemLibrary::BoxTraceSingle(World, PortalPosition, FVector(PortalPosition.X+5,PortalPosition.Y,PortalPosition.Z), FVector(7, GetPortalRadius(), GetPortalHeight()), OriginalHit.Actor->GetActorRotation(), PortalQueryChannel, true, IgnoreBox, EDrawDebugTrace::None, outHitBox, true);
		if (bHit) {
			return false;
		}

		TArray<FHitResult> outHitRight, outHitLeft, outHitUp, outHitDown;
		FVector tracePosRight = LocalHitLocation + FVector(-2, -GetPortalRadius(), 0);
		tracePosRight = UKismetMathLibrary::TransformLocation(CurrentWall->GetActorTransform(), tracePosRight);
		
		
		bool bCanPlaceRight = World->LineTraceMultiByChannel(outHitRight, InitialTracePoint, tracePosRight, ECollisionChannel::ECC_Visibility, Params);
		if (bCanPlaceRight) {
			for (int i = 0; i < outHitRight.Num(); ++i) {
				if (outHitRight[i].GetActor() != CurrentWall) {
					bCanPlaceRight = false;
					break;
				}
			}
		}
		
		UE_LOG(LogTemp, Error, TEXT("Can Place Right: %s"), *UKismetStringLibrary::Conv_BoolToString(bCanPlaceRight));
		//= World->LineTraceSingleByChannel(outHitRight, InitialTracePoint, tracePosRight, PortalWallCollision,Params);
		//DrawDebugLine(World, InitialTracePoint, tracePosRight, FColor::Cyan, false, 10.f);
		//DrawDebugSphere(World, tracePosRight, 22, 12, FColor::Purple, false, 10.f);



		//Left
		FVector tracePosLeft = LocalHitLocation + FVector(-2, GetPortalRadius(), 0);

		tracePosLeft = UKismetMathLibrary::TransformLocation(CurrentWall->GetActorTransform(), tracePosLeft);
		
		
		bool bCanPlaceLeft = World->LineTraceMultiByChannel(outHitLeft, InitialTracePoint, tracePosLeft, ECollisionChannel::ECC_Visibility, Params);
		if (bCanPlaceLeft) {
			for (int i = 0; i < outHitLeft.Num(); ++i) {
				if (outHitLeft[i].GetActor() != CurrentWall) {
					bCanPlaceLeft = false;
					break;
				}
			}
		}
		//bool bCanPlaceLeft = World->LineTraceSingleByChannel(outHitLeft, InitialTracePoint, tracePosLeft, PortalWallCollision, Params);
		//DrawDebugLine(World, InitialTracePoint, tracePosLeft, FColor::Cyan, false, 10.f);
		//DrawDebugSphere(World, tracePosLeft, 22, 12, FColor::Purple, false, 10.f);
		UE_LOG(LogTemp, Error, TEXT("Can Place Left: %s"), *UKismetStringLibrary::Conv_BoolToString(bCanPlaceLeft));

		//up
		FVector tracePosUp = LocalHitLocation + FVector(-2, 0, GetPortalHeight());

		tracePosUp = UKismetMathLibrary::TransformLocation(CurrentWall->GetActorTransform(), tracePosUp);
		

		bool bCanPlaceUp = World->LineTraceMultiByChannel(outHitUp, InitialTracePoint, tracePosUp, ECollisionChannel::ECC_Visibility, Params);
		if (bCanPlaceUp) {
			for (int i = 0; i < outHitUp.Num(); ++i) {
				if (outHitUp[i].GetActor() != CurrentWall) {
					bCanPlaceUp = false;
					break;
				}
			}
		}
		//bool bCanPlaceUp = World->LineTraceSingleByChannel(outHitUp, InitialTracePoint, tracePosUp, PortalWallCollision, Params);
		//DrawDebugLine(World, InitialTracePoint, tracePosUp, FColor::Cyan, false, 10.f);
		//DrawDebugSphere(World, tracePosUp, 22, 12, FColor::Purple, false, 10.f);

		UE_LOG(LogTemp, Error, TEXT("Can Place Up: %s"), *UKismetStringLibrary::Conv_BoolToString(bCanPlaceUp));

		FVector tracePosDown = LocalHitLocation + FVector(-2, 0, -GetPortalHeight());

		tracePosDown = UKismetMathLibrary::TransformLocation(CurrentWall->GetActorTransform(), tracePosDown);
		

		bool bCanPlaceDown = World->LineTraceMultiByChannel(outHitDown, InitialTracePoint, tracePosDown, ECollisionChannel::ECC_Visibility, Params);
		if (bCanPlaceDown) {
			for (int i = 0; i < outHitDown.Num(); ++i) {
				if (outHitDown[i].GetActor() != CurrentWall) {
					bCanPlaceDown = false;
					break;
				}
			}
		}

		UE_LOG(LogTemp, Error, TEXT("Can Place Down: %s"), *UKismetStringLibrary::Conv_BoolToString(bCanPlaceDown));

		return bCanPlaceDown && bCanPlaceUp && bCanPlaceRight && bCanPlaceLeft;
	}


	return false;
}

float UPortalManagerComponent::GetPortalRadius() {
	return correctRadiusPortal / 2;
}

float UPortalManagerComponent::GetPortalHeight()
{
	return correctHeightPortal / 2;
}

void UPortalManagerComponent::SetupPortalAs(APortalActor* Portal, bool bBlue)
{
	if (!Portal) {
		return;
	}
	Portal->SetPortalColor(bBlue? BlueColor : OrangeColor);
	if (bBlue) {
		PortalA = Portal;
		PortalA->SetRenderTexture(TextureBlue);
	}
	else {
		PortalB = Portal;
		PortalB->SetRenderTexture(TextureOrange);
	}
	if (PortalA && PortalB) {
		
		PortalA->SetPortalTarget(PortalB);
		PortalB->SetPortalTarget(PortalA);

		UTextureRenderTarget2D* TextureSecond = PortalB->GetRenderTexture();
		UTextureRenderTarget2D* TextureFirst = PortalA->GetRenderTexture();
		PortalA->SetViewTexture(TextureSecond);
		PortalB->SetViewTexture(TextureFirst);
		//	LinkPortals(PortalA, PortalB);
	}
}

void UPortalManagerComponent::LinkPortals(APortalActor* FirstPortal, APortalActor* SecondPortal)
{
	if (!FirstPortal || !SecondPortal) {
		return;
	}
	SecondPortal->SetPortalTarget(FirstPortal);
	FirstPortal->SetPortalTarget(SecondPortal);

	UTextureRenderTarget2D* TextureSecond = SecondPortal->GetRenderTexture();
	UTextureRenderTarget2D* TextureFirst = FirstPortal->GetRenderTexture();
	FirstPortal->SetViewTexture(TextureSecond);
	SecondPortal->SetViewTexture(TextureFirst);
}

UTextureRenderTarget2D* UPortalManagerComponent::GeneratePortalTexture()
{
	int32 CurrentSizeX = 1920;
	int32 CurrentSizeY = 1080;

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if(OwnerCharacter)
	{
		APlayerController* cont = Cast<APlayerController>(OwnerCharacter->Controller);
		if (cont) {
			cont->GetViewportSize(CurrentSizeX,CurrentSizeY);
		}
	}
	// Use a smaller size than the current 
	// screen to reduce the performance impact
	CurrentSizeX = FMath::Clamp(int(CurrentSizeX / 1.7), 128, 1920); //1920 / 1.5 = 1280
	CurrentSizeY = FMath::Clamp(int(CurrentSizeY / 1.7), 128, 1080);

	UTextureRenderTarget2D* PortalTexture;
	// Create new RTT
	PortalTexture = NewObject<UTextureRenderTarget2D>(this,UTextureRenderTarget2D::StaticClass(),*FString("PortalRenderTarget"));
	check(PortalTexture);

	PortalTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;
	PortalTexture->Filter = TextureFilter::TF_Bilinear;
	PortalTexture->SizeX = CurrentSizeX;
	PortalTexture->SizeY = CurrentSizeY;
	PortalTexture->ClearColor = FLinearColor::Black;
	PortalTexture->TargetGamma = 2.2f;
	PortalTexture->bNeedsTwoCopies = false;
	PortalTexture->AddressX = TextureAddress::TA_Clamp;
	PortalTexture->AddressY = TextureAddress::TA_Clamp;

	PortalTexture->bAutoGenerateMips = false;
	PortalTexture->UpdateResource();

	return PortalTexture;
}

/*
void UPortalManagerComponent::TryAddPortal(bool bPortalA, FVector PortalOrigin, APortalWall* Wall)
{
	if (!Wall) {
		return;
	}
	DrawDebugSphere(GetWorld(), PortalOrigin, 32.f, 12, FColor::Blue, false, 10.f);

	FVector RelativePortalOrigin = UKismetMathLibrary::InverseTransformLocation(Wall->GetActorTransform(), PortalOrigin);

	FVector MiddleManVector = ConstrainPortalToWall(RelativePortalOrigin.Y, RelativePortalOrigin.Z, Wall);

	FVector PortalPlacement = FVector(RelativePortalOrigin.X, MiddleManVector.Y, MiddleManVector.Z);
	PortalPlacement = UKismetMathLibrary::TransformLocation(Wall->GetActorTransform(), PortalPlacement);

	DrawDebugSphere(GetWorld(), PortalPlacement, 32.f, 12, FColor::Red, false, 10.f);

	//Check for portal collision here!
	//Cant spawn a second portal if one already exists
}
*/
/*
FVector UPortalManagerComponent::ConstrainPortalToWall(float PortalY, float PortalZ, APortalWall* Wall)
{
	float portalR = GetPortalRadius();
	float ClampedYValue = ClampPointToWall(PortalY, Wall->GetWallWidth(), GetPortalRadius());
	portalR = GetPortalHeight();
	float ClampedZValue = ClampPointToWall(PortalZ, Wall->GetWallHeight(), GetPortalHeight());

	FVector returnVal = FVector(0, ClampedYValue, ClampedZValue) * offsetMultiplier;

	UE_LOG(LogTemp, Error, TEXT("Portal Clamped Value: %s"), *returnVal.ToString());
	return returnVal;
}

float UPortalManagerComponent::ClampPointToWall(float point, float WallSize, float PortalRadius)
{
	float OffSet = (WallSize / 2) - PortalRadius;

	OffSet = OffSet - FMath::Abs(point);

	OffSet = FMath::Clamp(OffSet, -999999999999999.f, 0.f);

	if (point > 0.f) {
		OffSet *= -1.f;
	}

	float OffsetPoint = point - OffSet;
	UE_LOG(LogTemp, Error, TEXT("Offset Value: %f"), OffSet);
	return OffsetPoint;
}*/

