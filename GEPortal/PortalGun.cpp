// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalGun.h"
#include "GEPortalProjectile.h"
#include "PortalActor.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h" 

void APortalGun::ClearPortal(bool bPortalA)
{
	APortalActor* PortalToClear = bPortalA ? PortalA : PortalB;
	if (PortalToClear) {
		PortalToClear->Destroy();
	}
}

void APortalGun::ServerSpawnPickup_Implementation()
{
	if (!HasAuthority())return;
	UWorld* const World = GetWorld();
	if (World) {
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		World->SpawnActor<AActor>(SpawnAfterDeath, GetActorLocation(), FRotator::ZeroRotator, ActorSpawnParams);
	}
}

void APortalGun::FireProjectile(bool bBlue)
{

	if (ProjectileClass)
	{
		UWorld* const World = GetWorld();
		if (World != nullptr)
		{
			const FVector SpawnPoint = WeaponMuzzle->GetComponentLocation();
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Owner = this;
			APawn* OwningPawn = Cast<APawn>(GetOwner());
			if (OwningPawn) {
				ActorSpawnParams.Instigator = OwningPawn;
				FRotator ControlRotation = OwningPawn->GetControlRotation();
				AGEPortalProjectile* Projectile = World->SpawnActor<AGEPortalProjectile>(ProjectileClass, SpawnPoint, ControlRotation, ActorSpawnParams);
				if (Projectile) {
					Projectile->SetOwningWeapon(this);
					Projectile->SetPortalType(bBlue);
					Projectile->SetProjectileColor(bBlue ? BlueColor : OrangeColor);
				}
			}
			
		}
	}
}

void APortalGun::PlacePortalAt_Implementation(FHitResult OriginalHit, bool bBlue)
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

/*void APortalGun::OnOwnerDeath()
{
	Super::OnOwnerDeath();
	if (GetOwner()->HasAuthority()) {
		ServerSpawnPickup();
	}
}*/

bool APortalGun::IsFacingPortalWall(FVector StartPoint, FVector DirectionVector)
{
	return false;
}

void APortalGun::ClearAllPortals()
{
}

void APortalGun::BeginPlay()
{
}

void APortalGun::Fire()
{
	FireProjectile(true);
}

void APortalGun::FireSecond()
{
	FireProjectile(false);
}

bool APortalGun::CheckPortalPlacement(FHitResult OriginalHit, FVector& OutPosition)
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
		
		bool bHit = UKismetSystemLibrary::BoxTraceSingle(World, PortalPosition, FVector(PortalPosition.X + 5, PortalPosition.Y, PortalPosition.Z), FVector(7, GetPortalRadius(), GetPortalHeight()), OriginalHit.Actor->GetActorRotation(), PortalQueryChannel, true, IgnoreBox, EDrawDebugTrace::None, outHitBox, true);
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

float APortalGun::GetPortalRadius() {
	return correctRadiusPortal / 2;
}

float APortalGun::GetPortalHeight()
{
	return correctHeightPortal / 2;
}

void APortalGun::SetupPortalAs(APortalActor* Portal, bool bBlue)
{
	if (!Portal) {
		return;
	}
	Portal->SetPortalColor(bBlue ? BlueColor : OrangeColor);
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
	}
}

void APortalGun::LinkPortals(APortalActor* FirstPortal, APortalActor* SecondPortal)
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