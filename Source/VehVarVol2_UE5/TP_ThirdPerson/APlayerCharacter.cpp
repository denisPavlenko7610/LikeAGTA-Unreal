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
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

APlayerCharacter::APlayerCharacter()
{
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
    
    auto* MovementComponent = GetCharacterMovement();
    MovementComponent->bOrientRotationToMovement = true;
    MovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    MovementComponent->JumpZVelocity = 700.f;
    MovementComponent->AirControl = 0.35f;
    MovementComponent->MaxWalkSpeed = 500.f;
    MovementComponent->MinAnalogWalkSpeed = 20.f;
    MovementComponent->BrakingDecelerationWalking = 2000.f;
    MovementComponent->BrakingDecelerationFalling = 1500.0f;
    
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f;
    CameraBoom->bUsePawnControlRotation = true;
    
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    
    weaponSMComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSMComponent"));
    weaponSMComponent->SetupAttachment(GetMesh(), FName("WeaponSocket"));
    weaponSMComponent->SetHiddenInGame(true);
    
    _exitOffset = FVector(0.f, -200.f, 0.f);
    _initialArmLength = 300.0f;
    _targetArmLength = 150.0f;
    _aimLerpDurationS = 0.25f;
    _elapsedTimeS = 0.0f;
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
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::fireAnimation);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::stopFire);
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::aim);
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::stopAim);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error,
            TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."),
            *GetNameSafe(this));
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

    _initialArmLength = CameraBoom->TargetArmLength;
    _targetArmLength = 150.0f;
    _elapsedTimeS = 0.0f;
    IsAiming = true;

    GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::stopAim(const FInputActionValue& Value)
{
    if (!rifleEquipped)
        return;

    _initialArmLength = CameraBoom->TargetArmLength;
    _targetArmLength = 300.0f;
    _elapsedTimeS = 0.0f;
    IsAiming = false;

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

    FVector ExitLocation = _currentVehicle->GetActorLocation() + _currentVehicle->GetActorRotation().RotateVector(_exitOffset);
    SetActorLocation(ExitLocation);
    SetActorRotation(_currentVehicle->GetActorRotation());
    _currentVehicle = nullptr;
}

void APlayerCharacter::fireAnimation(const FInputActionValue& InputActionValue)
{
    if (!rifleEquipped)
        return;

    _canFire = false;

    playFireMontage(fireMontage);
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        AnimInstance->OnMontageEnded.AddUniqueDynamic(this, &APlayerCharacter::onMontageEnded);
    }
}

void APlayerCharacter::fire()
{
    float Length = 1000.f;
    FTransform SocketTransform = weaponSMComponent->GetSocketTransform("b_gun_muzzleflash");
    FVector Start = SocketTransform.GetLocation();
    FVector ForwardVector = SocketTransform.GetRotation().GetUpVector();
    FVector End = ForwardVector * Length + Start;

    FHitResult OutHit;
    FCollisionQueryParams CollisionParams;

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        OutHit,
        Start,
        End,
        ECC_Visibility,
        CollisionParams
    );

    if (bHit)
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1, 0, 1);

        if (OutHit.GetActor())
        {
            UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *OutHit.GetActor()->GetName());
        }
    }
    else
    {
        DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1, 0, 1);
    }
}

void APlayerCharacter::stopFire(const FInputActionValue& InputActionValue)
{
    if (!rifleEquipped)
        return;

    _canFire = true;
}

void APlayerCharacter::playFireMontage(UAnimMontage* Montage)
{
    if (!Montage)
        return;

    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        float PlayRate = 1.0f;
        AnimInstance->Montage_Play(Montage, PlayRate);
    }
}

void APlayerCharacter::onMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (_canFire)
        return;

    fireAnimation({});
}
