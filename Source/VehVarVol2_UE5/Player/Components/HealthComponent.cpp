#include "HealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

UHealthComponent::UHealthComponent()
{
	MaxHealth = 100.0f;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentHealth = MaxHealth;

	AActor* owner = GetOwner();
	if (owner)
	{
		owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::takeDamage);
	}
}

void UHealthComponent::takeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (bIsDead)
		return;
	
	if (Damage <= 0.0f || CurrentHealth <= 0.0f)
		return;
	
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	
	OnHealthChanged.Broadcast(this, CurrentHealth, Damage, DamageType, InstigatedBy, DamageCauser);

	if (CurrentHealth <= 0.0f)
		bIsDead = true;
}

void UHealthComponent::heal(float HealAmount)
{
	if (HealAmount <= 0.0f || CurrentHealth <= 0.0f)
		return;

	CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);

	OnHealthChanged.Broadcast(this, CurrentHealth, -HealAmount, nullptr,
		nullptr, nullptr);
}
