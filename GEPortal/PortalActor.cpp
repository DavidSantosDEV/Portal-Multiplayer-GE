// Fill out your copyright notice in the Description page of Project Settings.


#include "PortalActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/CapsuleComponent.h"
#include "Engine/TextureRenderTarget2D.h" 
#include "Kismet/GameplayStatics.h"
#include "GEPortalCharacter.h"
#include "Components/BoxComponent.h" 
#include "GEPortalCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h" 
#include "GameFramework/PlayerController.h" 
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
APortalActor::APortalActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PortalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PortalRoot"));
	RootComponent = PortalRoot;

	PortalCheat = CreateDefaultSubobject<USceneComponent>(TEXT("CheatComp"));
	PortalCheat->SetupAttachment(RootComponent);
	PortalCheat->SetRelativeRotation(FRotator(0, -180, 0));

	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
	CapsuleCollision->SetupAttachment(RootComponent);

	//TeleportCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Teleport Collision"));
	//TeleportCollision->SetupAttachment(RootComponent);

	PortalBackground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalBackground"));
	PortalBackground->SetupAttachment(RootComponent);
	PortalBorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalBorder"));
	PortalBorderMesh->SetupAttachment(RootComponent);

	SceneCaptureCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("Scene Capture Camera"));
	SceneCaptureCamera->SetupAttachment(RootComponent);
	SceneCaptureCamera->bOverride_CustomNearClippingPlane = true;
	SceneCaptureCamera->SetIsReplicated(true);
	bReplicates = true;
}

// Called when the game starts or when spawned
void APortalActor::BeginPlay()
{
	Super::BeginPlay();
    
   	CapsuleCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &APortalActor::OnBeginOverlap);
	CapsuleCollision->OnComponentEndOverlap.AddUniqueDynamic(this, &APortalActor::OnEndOverlap);

	//TeleportCollision->OnComponentBeginOverlap.AddUniqueDynamic(this, &APortalActor::OnBeginOverlapTeleport);
}

void APortalActor::CheckShouldTeleport(AGEPortalCharacter* ActorToTeleport)
{
	UWorld* const World = GetWorld();
	if (World) {

		FVector ActorDistance = ActorToTeleport->GetActorLocation() + (ActorToTeleport->GetVelocity() * World->GetDeltaSeconds());
		ActorDistance -= GetActorLocation(); //Minus the distance to portal
		ActorDistance = UKismetMathLibrary::Normal(ActorDistance);

		float fTrueDistance = UKismetMathLibrary::Dot_VectorVector(ActorDistance, GetActorForwardVector());
		float fDirection = UKismetMathLibrary::Dot_VectorVector(ActorToTeleport->GetLastMovementInputVector(), GetActorForwardVector());

		if (fTrueDistance<=0 && fDirection<=0) {
			TeleportActor(ActorToTeleport);
		}
	}
	
}

void APortalActor::TeleportActor(AGEPortalCharacter* ActorToTeleport)
{
	
	FVector RelativeActorVelocity = UKismetMathLibrary::InverseTransformDirection(ActorToTeleport->GetActorTransform(), ActorToTeleport->GetMovementComponent()->Velocity);
	
	FTransform RelativeTransform = UKismetMathLibrary::MakeRelativeTransform(ActorToTeleport->GetFirstPersonCameraComponent()->GetComponentTransform(), PortalCheat->GetComponentTransform());// UKismetMathLibrary::ConvertTransformToRelative(PortalCheat->GetComponentTransform(), ActorToTeleport->GetFirstPersonCameraComponent()->GetComponentTransform());
	RelativeTransform *= Target->GetActorTransform();

	FVector NewLocation = Target->GetActorForwardVector() * PortalPushForce;
	UE_LOG(LogTemp, Warning, TEXT(" % f"), PortalPushForce);
	FVector Mid = RelativeTransform.GetLocation() - ActorToTeleport->GetFirstPersonCameraComponent()->GetRelativeLocation();
	NewLocation = NewLocation + RelativeTransform.GetLocation();

	ActorToTeleport->SetActorLocation(NewLocation);

	AController* PController = ActorToTeleport->GetController();
	if (PController) {
		FRotator Rot = RelativeTransform.Rotator();
		Rot.Roll = 0;
		PController->SetControlRotation(Rot);

		FTransform VelocityTransform = FTransform(PController->GetControlRotation(), ActorToTeleport->GetActorLocation());
		FVector OutVelocity = UKismetMathLibrary::TransformDirection(VelocityTransform, RelativeActorVelocity);
		ActorToTeleport->GetMovementComponent()->Velocity = OutVelocity;
	}
	
	
}

void APortalActor::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (Target) {
		if (PlayersInPortal.Contains(OtherActor)) {
			return;
		}
		AGEPortalCharacter* player = Cast<AGEPortalCharacter>(OtherActor);
		if (player) {
			PlayersInPortal.AddUnique(player);
			OtherComp->SetCollisionProfileName("PawnPortalCol", true);
			UE_LOG(LogTemp, Warning, TEXT("EnteredPortal: %s"), *OtherComp->GetName());
		}
	}


}

void APortalActor::OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (Target) {
		if (PlayersInPortal.Contains(OtherActor)) {
			AGEPortalCharacter* player = Cast<AGEPortalCharacter>(OtherActor);
			if (player) {
				PlayersInPortal.Remove(player);
				OtherComp->SetCollisionProfileName("Pawn", true);
				UE_LOG(LogTemp, Warning, TEXT("Exited Portal: %s"), *OtherComp->GetName());
			}
		}

	}

}

// Called every frame
void APortalActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, GetWorld()->DeltaTimeSeconds, FColor::Purple, FString::Printf(TEXT("Player Name : %s"), *UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetName()));
	}
	//

	if(Target){
		
		APlayerCameraManager* PlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0); //GetWorld()->GetFirstLocalPlayerFromController()->GetPlayerController(GetWorld())->PlayerCameraManager;//  APlayerController::GetLocalPlayer()->PlayerCameraManager;// UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (PlayerCameraManager) {
			FTransform PlayerTransform = PlayerCameraManager->GetTransformComponent()->GetComponentTransform();

			FTransform Final = UKismetMathLibrary::MakeRelativeTransform(PlayerTransform, PortalCheat->GetComponentTransform()); // UKismetMathLibrary::ConvertTransformToRelative(PortalCheat->GetComponentTransform(), PlayerTransform);
			
			Target->SceneCaptureCamera->SetRelativeLocationAndRotation(Final.GetLocation(), Final.GetRotation());

			float NearClip = FVector::Distance(PlayerCameraManager->GetCameraLocation(), GetActorLocation()) + NearClipOffset;
			Target->SceneCaptureCamera->CustomNearClippingPlane = NearClip;
		}


		for (int i = 0; i < PlayersInPortal.Num(); ++i) {
			CheckShouldTeleport(PlayersInPortal[i]);
		}
	}
}
UTextureRenderTarget2D* APortalActor::GetRenderTexture()
{
	return SceneCaptureCamera->TextureTarget;
}

void APortalActor::SetPortalTarget_Implementation(APortalActor* newTarget)
{
	Target = newTarget;
}

void APortalActor::SetRenderTexture_Implementation(UTextureRenderTarget2D* RenderTexture)
{
	SceneCaptureCamera->TextureTarget = RenderTexture;
}

void APortalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APortalActor, Target);
}



