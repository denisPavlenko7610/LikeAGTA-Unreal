// 

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "ACar.generated.h"

class UInputAction;
class UInputMappingContext;
class USphereComponent;

UCLASS(Abstract)
class VEHVARVOL2_UE5_API AACar : public AWheeledVehiclePawn {
	GENERATED_BODY()

public:
	AACar();

	void PossessVehicle(AController* NewController);
	void UnpossessVehicle();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
	USphereComponent* CollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionRadius = 250.f;

};
