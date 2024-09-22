#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnHealthChangedSignature, UHealthComponent*, HealthComp, float, Health, float, HealthDelta,
	const class UDamageType*, DamageType, class AController*, InstigatedBy, AActor*, DamageCauser);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	UFUNCTION()
	void takeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	float getMaxHealth() { return MaxHealth; }

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CurrentHealth")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "CurrentHealth")
	float CurrentHealth;

public:
	UPROPERTY()
	FOnHealthChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintReadOnly, Category = "Health")
	bool bIsDead;
	
	float getHealth() const { return CurrentHealth; }
	void heal(float HealAmount);
};
