// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Citizen.generated.h"

class UHealthComponent;

UCLASS()
class VEHVARVOL2_UE5_API ACitizen : public ACharacter {
	GENERATED_BODY()

public:
	ACitizen();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* bloodParticle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UHealthComponent* HealthComponent;

	UFUNCTION()
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	virtual void onHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy,
	                             AActor* DamageCauser);
protected:
	virtual void BeginPlay() override;

private:
	void die();

	void spawnImpactParticles(FDamageEvent const& DamageEvent);

};
