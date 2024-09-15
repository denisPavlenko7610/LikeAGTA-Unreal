// 

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "VehicleInteraction.generated.h"

class APlayerCharacter;
class ACar;
/**
 * 
 */
UCLASS()
class VEHVARVOL2_UE5_API UVehicleInteraction : public UObject {
	GENERATED_BODY()

public:
	UVehicleInteraction();
	
	void enterVehicle(ACar* Vehicle);
	void exitVehicle();
	void init(APlayerCharacter* playerCharacter);

private:
	UPROPERTY()
	ACar* _currentVehicle;

	UPROPERTY()
	APlayerCharacter* _playerCharacter;

	FVector _exitOffset;
};
