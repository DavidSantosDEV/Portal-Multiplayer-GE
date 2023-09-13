// Copyright Epic Games, Inc. All Rights Reserved.

#include "GEPortalCharacter.h"
#include "GEPortalProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "WeaponBase.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h" 
#include "GameFramework/CharacterMovementComponent.h" 
#include "GEPortalGameMode.h"
#include "CustomPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AGEPortalCharacter

AGEPortalCharacter::AGEPortalCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	GetMesh()->bOwnerNoSee = true;
	GetMesh()->SetIsReplicated(true);

	GetMesh1P()->SetIsReplicated(true);

	/*FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));*/

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	bReplicates = true;
}

void AGEPortalCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (EndPlayReason== EEndPlayReason::Destroyed) {
		for (int i = 0; i < WeaponList.Num(); ++i) {
			if (WeaponList[i]) {
				WeaponList[i]->Destroy();
			}
			
		}
		WeaponList.Empty();
		SetReplicatedWeaponColor(FColor::Black);
		
		//RemoveAllWeapons();
	}
	
}

void AGEPortalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FirePrimary", IE_Pressed, this, &AGEPortalCharacter::FireWeaponPrimary);
	PlayerInputComponent->BindAction("FirePrimary", IE_Released, this, &AGEPortalCharacter::ReleaseWeaponPrimary);

	PlayerInputComponent->BindAction("FireSecondary", IE_Pressed, this, &AGEPortalCharacter::ServerFireWeaponSecondary);

	PlayerInputComponent->BindAction("NextWeapon", IE_Pressed, this, &AGEPortalCharacter::NextWeapon);
	PlayerInputComponent->BindAction("PreviousWeapon", IE_Pressed, this , &AGEPortalCharacter::PreviousWeapon);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AGEPortalCharacter::ReloadWeapon);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AGEPortalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGEPortalCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AGEPortalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AGEPortalCharacter::LookUpAtRate);
}

void AGEPortalCharacter::OnRep_WeaponIndex()
{
	if (WeaponList.IsValidIndex(WeaponIndex)) {
		UpdateWeaponUI(WeaponList[WeaponIndex]);
	}
}

void AGEPortalCharacter::UpdateWeaponUI(AWeaponBase* Weapon)
{
	if (!CastedController) {
		CastedController = Cast<ACustomPlayerController>(GetController());
		if (!CastedController) {
			return;
		}
	}
	CastedController->UpdateWeaponUI(Weapon);
}

void AGEPortalCharacter::RemoveAllWeapons()
{
	for (int i = 0; i < WeaponList.Num(); ++i) {
		WeaponList[i]->Destroy();
	}
	WeaponList.Empty();
	SetReplicatedWeaponColor(FColor::Black);
}

void AGEPortalCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//GetController()->SetReplicates(true);
	CreateDefaultWeapons();
	if (HasAuthority()) {
		HealthComp->OnDeathEvent.AddDynamic(this, &AGEPortalCharacter::CallOnDead);
	}
	//CreateWeaponUI();
	if (WeaponList.IsValidIndex(0)) {
		UpdateWeaponUI(WeaponList[0]);
	}
}

void AGEPortalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGEPortalCharacter, WeaponList);
	DOREPLIFETIME(AGEPortalCharacter, WeaponIndex);
}

void AGEPortalCharacter::GiveWeapon(TSubclassOf<AWeaponBase> WeaponClass, bool bAutoEquip)
{
	if (!HasAuthority())
	{
		return;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = this;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWeaponBase* Weapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, SpawnParams);
	if (Weapon) {

		Weapon->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
		WeaponList.AddUnique(Weapon);
		if (GEngine) {
			GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, FString::Printf(TEXT("%s with weapon %s"), *this->GetName(), *Weapon->GetName()));
		}
		Weapon->SetActorHiddenInGame(true);
		Weapon->SetActorEnableCollision(false);

		if (bAutoEquip) {
			SetWeaponActive(Weapon, WeaponList[WeaponIndex]);
		}
	}
}

bool AGEPortalCharacter::HasWeapon(TSubclassOf<AWeaponBase> WeaponInQuestion)
{
	if (!IsValid(WeaponInQuestion)) {
		return false;
	}
	for (int i = 0; i < WeaponList.Num(); ++i) {
		if (WeaponList[i]->GetClass() == WeaponInQuestion) {
			return true;
		}
	}
	return false;
}

AWeaponBase* AGEPortalCharacter::GetWeaponOfClass(TSubclassOf<AWeaponBase> WeaponClass)
{
	for (int i = 0; i < WeaponList.Num(); ++i)
	{
		if (WeaponList[i]->GetClass() == WeaponClass)
		{
			return WeaponList[i];
		}
	}
	return nullptr;
}

void AGEPortalCharacter::NextWeapon()
{
	if (HealthComp->GetIsAlive()) {
		if (WeaponList.Num() > 1) {
			ChangeWeapon(1);
		}
	}

	
}

void AGEPortalCharacter::PreviousWeapon()
{
	if (HealthComp->GetIsAlive()) {
		if (WeaponList.Num() > 1)
		{
			ChangeWeapon(-1);
		}
	}
}

void AGEPortalCharacter::ChangeWeapon_Implementation(int valueChange)
{
	if (HasAuthority()) {
		WeaponList[WeaponIndex]->OnFireReleased();
		int num = WeaponIndex + valueChange;
		if (num>WeaponList.Num()-1) {
			num = 0;
		}
		else if (num < 0) {
			num = WeaponList.Num()-1;
		}
		if (WeaponList.IsValidIndex(num)) {
			SetWeaponActive(WeaponList[num], WeaponList[WeaponIndex]);
		}

	}
}

void AGEPortalCharacter::SetWeaponActive(AWeaponBase* Weapon, AWeaponBase* PreviousWeapon)
{
	if (PreviousWeapon) {
		PreviousWeapon->SetActorHiddenInGame(true);
		PreviousWeapon->SetActorEnableCollision(false);
	}
	if (Weapon) {
		WeaponIndex = WeaponList.IndexOfByKey(Weapon);
		Weapon->SetActorHiddenInGame(false);
		Weapon->SetActorEnableCollision(true);
		SetReplicatedWeaponColor(Weapon->GetWeaponColor());
		UpdateWeaponUI(Weapon);
		//OnWeaponChanged.Broadcast(WeaponList[WeaponIndex]);
		//BroadCastWeaponChange();
		//OnWeaponChanged.Broadcast(Weapon);
	}

}

void AGEPortalCharacter::CreateDefaultWeapons()
{
	if (DefaultWeapons.Num() > 0) {
		for (int i = 0; i < DefaultWeapons.Num(); ++i)
		{
			GiveWeapon(DefaultWeapons[i], false);
		}
		SetWeaponActive(WeaponList[0], nullptr);
	}
}

/*void AGEPortalCharacter::CreateDefaultWeapons_Implementation()
{
	if (HasAuthority()) 
	{
		if (DefaultWeapons.Num()>0) {
			for (int i = 0; i < DefaultWeapons.Num(); ++i)
			{
				GiveWeapon(DefaultWeapons[i], false);
			}
			SetWeaponActive(WeaponList[0],nullptr);
		}
	}
}*/

void AGEPortalCharacter::FireWeaponPrimary()
{
	ServerFireWeaponPrimary();
}

void AGEPortalCharacter::ReleaseWeaponPrimary()
{
	ServerReleaseWeaponPrimary();
}

void AGEPortalCharacter::FireWeaponSecondary()
{
	if (!HealthComp->GetIsAlive())return;
}

void AGEPortalCharacter::ReloadWeapon_Implementation()
{
	if (HasAuthority()) {
		if (!HealthComp->GetIsAlive())return;
		if (WeaponList[WeaponIndex]) {
			WeaponList[WeaponIndex]->ReloadWeapon();
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Input



bool AGEPortalCharacter::GetIsLookingAtPortalWall()
{
	return bIsLookingAtPortalWall;
}

AWeaponBase* AGEPortalCharacter::GetPlayerCurrentWeapon()
{
	return WeaponList[WeaponIndex];
}

void AGEPortalCharacter::ServerFireWeaponPrimary_Implementation()
{
	if (HasAuthority()) {
		if (!HealthComp->GetIsAlive())return;
		if (WeaponList.IsValidIndex(WeaponIndex) && WeaponList[WeaponIndex]) {
			WeaponList[WeaponIndex]->OnFirePressed();
		}
	}
}

void AGEPortalCharacter::ServerReleaseWeaponPrimary_Implementation()
{
	if (HasAuthority()) {
		if (!HealthComp->GetIsAlive())return;
		if (WeaponList.IsValidIndex(WeaponIndex) && WeaponList[WeaponIndex]) {
			WeaponList[WeaponIndex]->OnFireReleased();
		}
	}
}

void AGEPortalCharacter::ServerFireWeaponSecondary_Implementation()
{
	if (HasAuthority()) {
		if (!HealthComp->GetIsAlive())return;
		if (WeaponList.IsValidIndex(WeaponIndex) && WeaponList[WeaponIndex]) {
			WeaponList[WeaponIndex]->OnFireSecondPressed();
		}
	}
}




void AGEPortalCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AGEPortalCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AGEPortalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AGEPortalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AGEPortalCharacter::CallOnDead(AController* MyKiller)
{
	GetMovementComponent()->StopMovementImmediately();
	AGEPortalGameMode* GameMode = (AGEPortalGameMode*)GetWorld()->GetAuthGameMode();
	if (GameMode) {
		GameMode->OnPlayerEliminated(this, GetController(), MyKiller);
	}
	
}

void AGEPortalCharacter::OnDieEvent_Implementation(AController* MyKiller)
{
	for (int i = 0; i < WeaponList.Num(); ++i) {
		WeaponList[i]->OnOwnerDeath();
	}

	GetMesh()->SetSimulatePhysics(true);

	GetCharacterMovement()->DisableMovement();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh1P()->SetIsReplicated(true);
	GetMesh1P()->SetVisibility(false, true);
	GetMesh()->SetCollisionProfileName("Ragdoll"); //Fixed collision name so...
	//GetMesh()->UpdateCollisionProfile();
}
