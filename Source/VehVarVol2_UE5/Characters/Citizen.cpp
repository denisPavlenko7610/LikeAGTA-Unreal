// 


#include "Citizen.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


ACitizen::ACitizen(): _isDead(false)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACitizen::BeginPlay()
{
	Super::BeginPlay();
}

float ACitizen::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (_isDead)
		return -1;
	
	float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (actualDamage <= 0.0f)
		actualDamage = DamageAmount;
	
	_currentHealth -= actualDamage;
	_currentHealth = FMath::Clamp(_currentHealth, 0.0f, _maxHealth);
	
	FString HealthMessage = FString::Printf(TEXT("Current health is: %f"), _currentHealth);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, HealthMessage);

	spawnImpactParticles(DamageEvent);

	if (_currentHealth <= 0)
		die();
	
	return actualDamage;
}

void ACitizen::die()
{
	_isDead = true;
	GetMesh()->SetSimulatePhysics(true);
}

void ACitizen::spawnImpactParticles(FDamageEvent const& DamageEvent)
{
	if (!DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		return;
	
	const FPointDamageEvent* pointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
	FHitResult hitResult = pointDamageEvent->HitInfo;
	
	if (UParticleSystemComponent* pointEffect = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		bloodParticle,
		hitResult.ImpactPoint,
		hitResult.Normal.ToOrientationRotator(),
		true,
		EPSCPoolMethod::AutoRelease
	))
	{
		float scale = 0.5f;
		pointEffect->SetWorldScale3D(FVector(scale));
	}
}

