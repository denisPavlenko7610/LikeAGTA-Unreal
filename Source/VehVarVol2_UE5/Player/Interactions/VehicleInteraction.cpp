// 


#include "VehicleInteraction.h"

#include "GameFramework/PawnMovementComponent.h"
#include "VehVarVol2_UE5/Cars/ACar.h"
#include "VehVarVol2_UE5/Player/APlayerCharacter.h"

void UVehicleInteraction::init(APlayerCharacter* playerCharacter)
{
	_playerCharacter = playerCharacter;
}

UVehicleInteraction::UVehicleInteraction(): _playerCharacter(nullptr)
{
	_currentVehicle = nullptr;
	_exitOffset = FVector(0.f, -200.f, 0.f);
}

void UVehicleInteraction::enterVehicle(ACar* Vehicle)
{
	if (!Vehicle)
		return;

	_currentVehicle = Vehicle;
	_playerCharacter->GetMovementComponent()->StopMovementImmediately();
	_playerCharacter->SetActorHiddenInGame(true);
	_playerCharacter->SetActorEnableCollision(false);
	Vehicle->possessVehicle(_playerCharacter);
}

void UVehicleInteraction::exitVehicle()
{
	if (!_currentVehicle)
		return;

	_playerCharacter->SetActorHiddenInGame(false);
	_playerCharacter->SetActorEnableCollision(true);

	FVector ExitLocation = _currentVehicle->GetActorLocation() + _currentVehicle->GetActorRotation().RotateVector(_exitOffset);
	_playerCharacter->SetActorLocation(ExitLocation);
	_playerCharacter->SetActorRotation(_currentVehicle->GetActorRotation());
	_currentVehicle = nullptr;
}
