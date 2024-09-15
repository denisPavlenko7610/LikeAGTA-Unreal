// 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Citizen.generated.h"

UCLASS()
class VEHVARVOL2_UE5_API ACitizen : public ACharacter {
	GENERATED_BODY()

public:
	ACitizen();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UParticleSystem* bloodParticle;
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

protected:
	virtual void BeginPlay() override;

private:
	float _maxHealth = 100.f;
	float _currentHealth = 100.f;
	bool _isDead;

	void die();
	void spawnImpactParticles(FDamageEvent const& DamageEvent);

};
