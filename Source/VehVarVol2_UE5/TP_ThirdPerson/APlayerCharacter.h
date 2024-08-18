#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "APlayerCharacter.generated.h"

class AACar;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USkeletalMeshComponent;
class UAnimMontage;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* weaponSMComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EnterAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* getWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	bool rifleEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	bool IsAiming;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* fireMontage;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	APlayerCharacter();
	void exitVehicle();
	void fire();

protected:
	void move(const FInputActionValue& Value);
	void look(const FInputActionValue& Value);
	void toggleWeapon(const FInputActionValue& Value);
	void aim(const FInputActionValue& Value);
	void stopAim(const FInputActionValue& Value);
	void updateAimLerp();
	void interact();
	
	void enterVehicle(AACar* Vehicle);

	UFUNCTION()
	void fireAnimation(const FInputActionValue& InputActionValue);
	void stopFire(const FInputActionValue& InputActionValue);
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	AACar* _currentVehicle;

	FVector _exitOffset;

	float _initialArmLength;
	float _targetArmLength;
	float _aimLerpDurationS;
	float _elapsedTimeS;
	FTimerHandle LerpTimerHandle;

	bool _canFire;

	void playFireMontage(UAnimMontage* montage);
	
	UFUNCTION()
	void onMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};

