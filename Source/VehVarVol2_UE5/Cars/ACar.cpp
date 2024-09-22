
#include "ACar.h"

#include "EnhancedInputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PawnMovementComponent.h"
#include "VehVarVol2_UE5/Player/APlayerCharacter.h"
#include "VehVarVol2_UE5/Player/Components/HealthComponent.h"
#include "VehVarVol2_UE5/Player/Interactions/VehicleInteraction.h"

ACar::ACar()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(CollisionRadius);
	SetRootComponent(GetMesh());

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ACar::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ACar::onHealthChanged);
	}
}

float ACar::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float actualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (HealthComponent)
	{
		HealthComponent->takeDamage(this, actualDamage, DamageEvent.DamageTypeClass
			? DamageEvent.DamageTypeClass.GetDefaultObject() : nullptr, EventInstigator, DamageCauser);
	}

	return actualDamage;
}

void ACar::onHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	FString HealthMessage = FString::Printf(TEXT("Current health is: %f"), Health);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, HealthMessage);
	
	if (Health <= 0)
		destroy();
}

void ACar::destroy()
{
	UE_LOG(LogTemp, Warning, TEXT("%s has died!"), *GetName());
	
	if (AController* controller = GetController())
	{
		controller->UnPossess();
	}
}

void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Started, this, &ThisClass::unpossessVehicle);
	}
}

void ACar::possessVehicle(APlayerCharacter* PlayerCharacter)
{
	_playerCharacter = PlayerCharacter;
	if (AController* controller = PlayerCharacter->GetController())
	{
		controller->Possess(this);
	}
}

void ACar::unpossessVehicle()
{
	if (!canAllowExit())
		return;
	
	if (!_playerCharacter)
		return;

	_playerCharacter->getVehicleInteraction()->exitVehicle();
	stopVehicle();
	GetController()->Possess(_playerCharacter);
	_playerCharacter = nullptr;
}

bool ACar::canAllowExit()
{
	double speed = GetVelocity().Size();
	return speed < _speedExitLimit;
}

void ACar::stopVehicle()
{
	GetMovementComponent()->StopMovementImmediately();
}
