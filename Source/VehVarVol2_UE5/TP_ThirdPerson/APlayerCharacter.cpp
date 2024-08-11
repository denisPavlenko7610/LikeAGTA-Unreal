// Copyright Epic Games, Inc. All Rights Reserved.

#include "APlayerCharacter.h"

#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SkeletalMeshComponent.h"
#include "../Cars/ACar.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APlayerCharacter::APlayerCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	_exitOffset = FVector(0.f, -200.f, 0.f);
	
	_initialArmLength = 300.0f;
	_targetArmLength = 150.0f;
	_aimLerpDurationS = 0.25f;
	_elapsedTimeS = 0.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = _initialArmLength;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	weaponSMComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("weaponSMComponent"));
	weaponSMComponent->SetupAttachment(GetMesh(), FName("WeaponSocket"));
	weaponSMComponent->SetHiddenInGame(true);
	
	_currentVehicle = nullptr;
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::look);
		EnhancedInputComponent->BindAction(EnterAction, ETriggerEvent::Started, this, &ThisClass::interact);
		EnhancedInputComponent->BindAction(getWeaponAction, ETriggerEvent::Started, this, &ThisClass::toggleWeapon);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::fire);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::stopFire);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::aim);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::stopAim);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
			TEXT(
				"'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."
			), *GetNameSafe(this));
	}
}

void APlayerCharacter::move(const FInputActionValue& Value)
{
	if (!Controller)
		return;
	
	FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void APlayerCharacter::look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller == nullptr)
		return;
	
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void APlayerCharacter::toggleWeapon(const FInputActionValue& Value)
{
	rifleEquipped = !rifleEquipped;
	weaponSMComponent->SetHiddenInGame(!rifleEquipped);
}

void APlayerCharacter::aim(const FInputActionValue& Value)
{
	if (!rifleEquipped)
		return;
	
	_initialArmLength = 300;
	_targetArmLength = 150;
	_elapsedTimeS = 0.0f;
	isAiming = true;
	
	GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::stopAim(const FInputActionValue& Value)
{
	if (!rifleEquipped)
		return;

	_initialArmLength = CameraBoom->TargetArmLength;
	_targetArmLength = 300;
	_elapsedTimeS = 0.0f;
	isAiming = false;
	
	GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::updateAimLerp()
{
	_elapsedTimeS += GetWorld()->GetDeltaSeconds();
	float Alpha = FMath::Clamp(_elapsedTimeS / _aimLerpDurationS, 0.0f, 1.0f);
	float NewArmLength = FMath::Lerp(_initialArmLength, _targetArmLength, Alpha);
	CameraBoom->TargetArmLength = NewArmLength;

	if (Alpha >= 1.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(LerpTimerHandle);
	}
}

void APlayerCharacter::interact()
{
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapResults,
		GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_Vehicle),
		FCollisionShape::MakeSphere(300.0f),
		QueryParams
	);

	if (!bHit)
		return;

	for (const FOverlapResult& Overlap : OverlapResults)
	{
		if (AACar* Car = Cast<AACar>(Overlap.GetActor()))
		{
			enterVehicle(Car);
			return;
		}
	}
}

void APlayerCharacter::enterVehicle(AACar* Vehicle)
{
	if (!Vehicle)
		return;
	
	_currentVehicle = Vehicle;
	GetMovementComponent()->StopMovementImmediately();
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	Vehicle->possessVehicle(this);
}

void APlayerCharacter::exitVehicle()
{
	if (!_currentVehicle)
		return;
		
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	FVector exitLocation = _currentVehicle->GetActorLocation() + _currentVehicle->GetActorRotation().RotateVector(_exitOffset);
	SetActorLocation(exitLocation);
	SetActorRotation(_currentVehicle->GetActorRotation());
	_currentVehicle = nullptr;
}

void APlayerCharacter::fire(const FInputActionValue& InputActionValue)
{
	if (!rifleEquipped)
		return;

	_isFiring = true;

	UAnimMontage* montage = fireMontage;
	montage->bLoop = true;
	playFireMontage(fireMontage);
	if (UAnimInstance* animInstance = GetMesh()->GetAnimInstance())
	{
		animInstance->OnMontageEnded.AddUniqueDynamic(this, &APlayerCharacter::onMontageEnded);
	}
}

void APlayerCharacter::stopFire(const FInputActionValue& InputActionValue)
{
	if (!rifleEquipped)
		return;

	_isFiring = false;
}

void APlayerCharacter::playFireMontage(UAnimMontage* montage)
{
	if (!montage)
		return;

	if (UAnimInstance* animInstance = GetMesh()->GetAnimInstance())
	{
		float playRate = 1.0f;
		animInstance->Montage_Play(montage, playRate);
	}
}

void APlayerCharacter::onMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!_isFiring)
		return;

	fire({});
}
