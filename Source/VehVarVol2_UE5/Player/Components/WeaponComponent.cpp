#include "WeaponComponent.h"

#include "HealthComponent.h"
#include "InputActionValue.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "VehVarVol2_UE5/Effects/Audio/AudioPlayer.h"
#include "VehVarVol2_UE5/Effects/Camera/UCameraShake.h"
#include "VehVarVol2_UE5/Player/APlayerCharacter.h"
 #include "Engine/DamageEvents.h"
#include "VehVarVol2_UE5/Characters/Citizen.h"

void UWeaponComponent::fireAnimation(const FInputActionValue& InputActionValue)
{
	if (!rifleEquipped)
		return;

	_canFire = false;

	playFireMontage(fireMontage);
	if (UAnimInstance* AnimInstance = _playerCharacter->GetMesh()->GetAnimInstance())
	{
		AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &UWeaponComponent::onMontageEnded);
	}
}

void UWeaponComponent::init(APlayerCharacter* playerCharacter)
{
	_playerCharacter = playerCharacter;
}

void UWeaponComponent::fire()
{
	constexpr float length = 1000.f;
	const FName socketName = "Muzzle";
    
	FVector cameraWorldLocation = _playerCharacter->FollowCamera->GetComponentLocation();
	FVector forwardVector = _playerCharacter->FollowCamera->GetForwardVector();

	FVector startMuzzlePosition;
	FVector socketForwardVector;
	_playerCharacter->getSocketTransformAndVectors(socketName, startMuzzlePosition, socketForwardVector);
    
	FVector endPosition = forwardVector * length + cameraWorldLocation;

	spawnFireEffect(socketName, startMuzzlePosition, forwardVector);
	handleFireSound();
    
	FHitResult hitResult;
	if (performLineTrace( cameraWorldLocation, endPosition, hitResult))
	{
		handleHit(hitResult);
	}
    
	applyCameraShake();
}

bool UWeaponComponent::performLineTrace(const FVector& start, const FVector& end, FHitResult& outHit) const
{
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(_playerCharacter);
	
	return GetWorld()->LineTraceSingleByChannel(
		outHit,
		start,
		end,
		ECC_Visibility,
		collisionParams
	);
}

void UWeaponComponent::handleHit(const FHitResult& hitResult)
{
	if (!impactParticle)
		return;
	
	if (!hitResult.GetActor())
		return;
	
	UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *hitResult.GetActor()->GetName());

	FPointDamageEvent PointDamageEvent;
	PointDamageEvent.HitInfo = hitResult;
	
	AActor* hitActor = hitResult.GetActor();

	if (!hitActor)
		return;

	if (hitActor->FindComponentByClass<UHealthComponent>())
	{
		hitResult.GetActor()->TakeDamage(DamageAmount, PointDamageEvent, GetOwner()->GetInstigatorController(), _playerCharacter);
	} else
	{
		if (UParticleSystemComponent* pointEffect = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		impactParticle,
		hitResult.ImpactPoint,
		hitResult.Normal.ToOrientationRotator(),
		true,
		EPSCPoolMethod::AutoRelease))
		{
			float scale = 0.5f;
			pointEffect->SetWorldScale3D(FVector(scale));
		}
	}
}

void UWeaponComponent::handleFireSound() const
{
	UAudioPlayer::playMetaSoundAtLocation(GetWorld(), _playerCharacter->GetActorLocation(), AudioList::fireSound);
}

void UWeaponComponent::spawnFireEffect(FName socketName, FVector& location, FVector& direction)
{
	if (!fireParticle)
		return;
    
	float vectorLength = 10.f;
	FVector adjustedLocation = location + direction * vectorLength;
	UParticleSystemComponent* fireEffect = UGameplayStatics::SpawnEmitterAttached(
		fireParticle,
		this,
		socketName,
		adjustedLocation,
		direction.Rotation(),
		EAttachLocation::KeepWorldPosition,
		true,
		EPSCPoolMethod::AutoRelease
	);

	if (!fireEffect)
		return;
    
	float scale = 0.15f;
	fireEffect->SetWorldScale3D(FVector(scale));
}

void UWeaponComponent::applyCameraShake()
{
	if (APlayerController* playerController = Cast<APlayerController>(_playerCharacter->GetController()))
	{
		playerController->PlayerCameraManager->StartCameraShake(UCameraShake::StaticClass(), 1.0f);
	}
}

void UWeaponComponent::stopFire(const FInputActionValue& inputActionValue)
{
	if (!rifleEquipped)
		return;

	_canFire = true;
}

void UWeaponComponent::playFireMontage(UAnimMontage* montage)
{
	if (!montage)
		return;

	if (UAnimInstance* AnimInstance = _playerCharacter->GetMesh()->GetAnimInstance())
	{
		float PlayRate = 1.0f;
		AnimInstance->Montage_Play(montage, PlayRate);
	}
}

void UWeaponComponent::onMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (_canFire)
		return;
	
	fireAnimation({});
}

void UWeaponComponent::toggleWeapon(const FInputActionValue& Value)
{
	rifleEquipped = !rifleEquipped;
	SetHiddenInGame(!rifleEquipped);
}