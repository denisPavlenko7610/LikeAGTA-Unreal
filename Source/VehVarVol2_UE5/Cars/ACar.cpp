
#include "ACar.h"

#include "Components/SphereComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "VehVarVol2_UE5/TP_ThirdPerson/APlayerCharacter.h"

AACar::AACar()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(CollisionRadius);
	RootComponent = CollisionComponent;
}

void AACar::PossessVehicle(AController* NewController)
{
	if (NewController)
	{
		NewController->Possess(this);
	}
}

void AACar::UnpossessVehicle()
{
	if (Controller)
	{
		Controller->UnPossess();
	}
}
