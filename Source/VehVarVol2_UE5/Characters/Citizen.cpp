// 


#include "Citizen.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "VehVarVol2_UE5/Player/Components/HealthComponent.h"


ACitizen::ACitizen()
{
	PrimaryActorTick.bCanEverTick = false;
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ACitizen::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACitizen::onHealthChanged);
	}
}

float ACitizen::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	spawnImpactParticles(DamageEvent);

	if (HealthComponent)
	{
		HealthComponent->takeDamage(this, ActualDamage, DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass.GetDefaultObject() : nullptr, EventInstigator, DamageCauser);
	}

	return ActualDamage;
}

void ACitizen::onHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	FString HealthMessage = FString::Printf(TEXT("Current health is: %f"), Health);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, HealthMessage);
	
	if (Health <= 0)
		die();
}

void ACitizen::die()
{
	UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());
	
	GetMovementComponent()->StopMovementImmediately();
	if (AController* controller = GetController())
	{
		controller->UnPossess();
	}

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

