
#include "ACar.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "VehVarVol2_UE5/TP_ThirdPerson/APlayerCharacter.h"

AACar::AACar()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(CollisionRadius);
	RootComponent = CollisionComponent;
}

void AACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Triggered, this, &ThisClass::exitFromCar);
	}
	else {
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AACar::poses()
{
	if (AController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		_insideCar = true;
		PlayerController->Possess(this);
	}
}

void AACar::exitFromCar()
{
	if (!_insideCar) {
		return;
	}

	_insideCar = false;
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController && PlayerCharacter)
	{
		PlayerController->Possess(PlayerCharacter);
		float lengthOfVectorToSpawn = 5.f;
		PlayerCharacter->SetActorLocation(GetActorLocation() + FVector::LeftVector * lengthOfVectorToSpawn);
		PlayerCharacter->setMeshVisibility(true);
	}
}
