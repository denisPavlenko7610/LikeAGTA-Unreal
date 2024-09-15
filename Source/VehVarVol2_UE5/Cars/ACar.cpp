
#include "ACar.h"

#include "EnhancedInputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VehVarVol2_UE5/Player/APlayerCharacter.h"
#include "VehVarVol2_UE5/Player/Interactions/VehicleInteraction.h"

ACar::ACar()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(CollisionRadius);
	SetRootComponent(GetMesh());
}

void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Started, this, &ThisClass::unpossessVehicle);
	}
}

void ACar::possessVehicle(APlayerCharacter* PlayerCharacter)
{
	_playerCharacter = PlayerCharacter;
	if (AController* controller = PlayerCharacter->GetController())
	{
		controller->Possess(this);
	}
}

void ACar::unpossessVehicle()
{
	if (!canAllowExit())
		return;
	
	if (!_playerCharacter)
		return;

	_playerCharacter->getVehicleInteraction()->exitVehicle();
	stopVehicle();
	GetController()->Possess(_playerCharacter);
	_playerCharacter = nullptr;
}

bool ACar::canAllowExit()
{
	double speed = GetVelocity().Size();
	return speed < _speedExitLimit;
}

void ACar::stopVehicle()
{
	GetMovementComponent()->StopMovementImmediately();
}
