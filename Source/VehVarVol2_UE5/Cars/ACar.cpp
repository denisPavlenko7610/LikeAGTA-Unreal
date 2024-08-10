
#include "ACar.h"

#include "EnhancedInputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VehVarVol2_UE5/TP_ThirdPerson/APlayerCharacter.h"

AACar::AACar()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(CollisionRadius);
	SetRootComponent(GetMesh());
}

void AACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Started, this, &ThisClass::unpossessVehicle);
	}
}

void AACar::possessVehicle(APlayerCharacter* PlayerCharacter)
{
	_playerCharacter = PlayerCharacter;
	if (AController* controller = PlayerCharacter->GetController())
	{
		controller->Possess(this);
	}
}

void AACar::unpossessVehicle()
{
	if (!canAllowExit())
		return;
	
	if (!_playerCharacter)
		return;

	_playerCharacter->exitVehicle();
	stopVehicle();
	GetController()->Possess(_playerCharacter);
	_playerCharacter = nullptr;
}

bool AACar::canAllowExit()
{
	double speed = GetVelocity().Size();
	return speed < _speedExitLimit;
}

void AACar::stopVehicle()
{
	GetMovementComponent()->StopMovementImmediately();
}
