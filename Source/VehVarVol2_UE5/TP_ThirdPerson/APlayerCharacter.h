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
class UUserWidget;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	USkeletalMeshComponent* weaponComponent;
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* fireParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* impactParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> hudWidget;

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	APlayerCharacter();
	virtual void BeginPlay() override;

	void exitVehicle();
	void fire();
	void handleFireSound() const;
	void applyCameraShake();
	void getSocketTransformAndVectors(const FName& socketName, FVector& outStart, FVector& outForwardVector) const;

protected:
	void move(const FInputActionValue& Value);
	void look(const FInputActionValue& Value);
	void toggleWeapon(const FInputActionValue& Value);
	void setSafeRotation();
	void aim(const FInputActionValue& Value);
	void setFreeRotation();
	void stopAim(const FInputActionValue& Value);
	void updateAimLerp();
	void interact();
	
	void enterVehicle(AACar* Vehicle);

	UFUNCTION()
	void fireAnimation(const FInputActionValue& InputActionValue);

	void handleHit(const FHitResult& hitResult);
	void stopFire(const FInputActionValue& InputActionValue);
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void createHUD();

	UFUNCTION(BlueprintCallable)
	FRotator getAimRotation();

private:
	UPROPERTY()
	AACar* _currentVehicle;

	FVector _exitOffset;

	float _initialArmLength;
	float _targetArmLength;
	float _aimLerpDurationS;
	float _elapsedTimeS;
	FVector2d _rightOffset;
	FTimerHandle LerpTimerHandle;

	bool _canFire;

	void playFireMontage(UAnimMontage* montage);
	
	UFUNCTION()
	void onMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void spawnFireEffect(FName socketName, FVector& location, FVector& direction);
	bool performLineTrace(const FVector& start, const FVector& end, FHitResult& outHit) const;
};

