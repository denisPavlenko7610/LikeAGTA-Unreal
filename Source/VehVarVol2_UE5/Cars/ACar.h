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
	//void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);
	//void Interact();
	void PossessVehicle(AController* NewController);
	void UnpossessVehicle();

	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	//void poses();
	//void exitFromCar();

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	// UInputMappingContext* DefaultMappingContext;
	//
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	// UInputAction* EnterAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
	USphereComponent* CollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionRadius = 250.f;

private:
	//bool _insideCar;
};
