// 

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/Object.h"
#include "WeaponComponent.generated.h"

class UCameraComponent;
class APlayerCharacter;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class VEHVARVOL2_UE5_API UWeaponComponent : public USkeletalMeshComponent {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Setup")
	float DamageAmount = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* fireMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* fireParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* impactParticle;

	void init(APlayerCharacter* playerCharacter);
	void fire();
	
	void applyCameraShake();

	void stopFire(const FInputActionValue& inputActionValue);
	void playFireMontage(UAnimMontage* montage);
	void handleFireSound() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Settings)
	bool rifleEquipped;

	void handleHit(const FHitResult& hitResult);

	UFUNCTION()
	void fireAnimation(const FInputActionValue& InputActionValue);

	UFUNCTION()
	void onMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void spawnFireEffect(FName socketName, FVector& location, FVector& direction);
	void toggleWeapon(const FInputActionValue& Value);

private:
	bool _canFire;

	UPROPERTY()
	APlayerCharacter* _playerCharacter;

	bool performLineTrace(const FVector& start, const FVector& end, FHitResult& outHit) const;
};
