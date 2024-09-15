// 

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "ACar.generated.h"

class APlayerCharacter;
class UInputAction;
class UInputMappingContext;
class USphereComponent;

UCLASS(Abstract)
class VEHVARVOL2_UE5_API ACar : public AWheeledVehiclePawn {
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EnterAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Collision")
	USphereComponent* CollisionComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	float CollisionRadius = 250.f;

	ACar();
	
	void possessVehicle(APlayerCharacter* PlayerCharacter);
	void unpossessVehicle();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	bool canAllowExit();
	void stopVehicle();

	UPROPERTY()
	APlayerCharacter* _playerCharacter;

	float _speedExitLimit = 120.f;
};
