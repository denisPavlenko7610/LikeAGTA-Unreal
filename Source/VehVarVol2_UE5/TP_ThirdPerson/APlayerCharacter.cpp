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
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "VehVarVol2_UE5/Effects/Audio/AudioPlayer.h"
#include "VehVarVol2_UE5/Effects/Camera/UCameraShake.h"
#include "Blueprint/UserWidget.h"

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

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f;
    CameraBoom->bUsePawnControlRotation = true;
    
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    
    weaponComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSMComponent"));
    weaponComponent->SetupAttachment(GetMesh(), FName("WeaponSocket"));
    weaponComponent->SetHiddenInGame(true);
    
    _exitOffset = FVector(0.f, -200.f, 0.f);
    _initialArmLength = 300.0f;
    _targetArmLength = 150.0f;
    _aimLerpDurationS = 0.25f;
    _elapsedTimeS = 0.0f;
    _currentVehicle = nullptr;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    createHUD();
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

void APlayerCharacter::createHUD()
{
    if (hudWidget == nullptr)
        return;
    
    UUserWidget* hud = CreateWidget<UUserWidget>(GetWorld(), hudWidget);
    if (hud == nullptr)
        return;

    hud->AddToViewport();
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
    weaponComponent->SetHiddenInGame(!rifleEquipped);
}

void APlayerCharacter::aim(const FInputActionValue& Value)
{
    if (!rifleEquipped)
        return;

    _initialArmLength = CameraBoom->TargetArmLength;
    _rightOffset = FVector2d(15,50);
    _targetArmLength = 150.0f;
    _elapsedTimeS = 0.0f;
    IsAiming = true;

    //setSafeRotation();
    
    GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::stopAim(const FInputActionValue& Value)
{
    if (!rifleEquipped)
        return;

    _initialArmLength = CameraBoom->TargetArmLength;
    _rightOffset = FVector2d(0,0);
    _targetArmLength = 300.0f;
    _elapsedTimeS = 0.0f;
    IsAiming = false;

    //setFreeRotation();
    
    GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::setSafeRotation()
{
    UCharacterMovementComponent* characterMovement = GetCharacterMovement();
    characterMovement->bOrientRotationToMovement = false;
    characterMovement->bUseControllerDesiredRotation = true;
}

void APlayerCharacter::setFreeRotation()
{
    UCharacterMovementComponent* characterMovement = GetCharacterMovement();
    characterMovement->bOrientRotationToMovement = true;
    characterMovement->bUseControllerDesiredRotation = false;
}

void APlayerCharacter::updateAimLerp()
{
    _elapsedTimeS += GetWorld()->GetDeltaSeconds();
    float alpha = FMath::Clamp(_elapsedTimeS / _aimLerpDurationS, 0.0f, 1.0f);
    float newArmLength = FMath::Lerp(_initialArmLength, _targetArmLength, alpha);
    float newRightOffset= FMath::Lerp(_rightOffset.X, _rightOffset.Y, alpha);
    
    CameraBoom->TargetArmLength = newArmLength;
    FVector cameraBoomLocation = CameraBoom->GetRelativeLocation();
    cameraBoomLocation.Y = newRightOffset;
    CameraBoom->SetRelativeLocation(cameraBoomLocation);
    
    if (alpha >= 1.f)
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
    constexpr float length = 1000.f;
    const FName socketName = "Muzzle";
    
    FVector cameraWorldLocation = FollowCamera->GetComponentLocation();
    FVector forwardVector = FollowCamera->GetForwardVector();

    FVector startMuzzlePosition;
    FVector socketForwardVector;
    getSocketTransformAndVectors(socketName, startMuzzlePosition, socketForwardVector);
    
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

void APlayerCharacter::handleFireSound() const
{
    UAudioPlayer::playMetaSoundAtLocation(GetWorld(), GetActorLocation(), AudioList::fireSound);
}

void APlayerCharacter::applyCameraShake()
{
    if (APlayerController* playerController = GetWorld()->GetFirstPlayerController())
    {
        playerController->PlayerCameraManager->StartCameraShake(UCameraShake::StaticClass(), 1.0f);
    }
}

void APlayerCharacter::getSocketTransformAndVectors(const FName& socketName, FVector& outStart, FVector& outForwardVector) const
{
    FTransform socketTransform = weaponComponent->GetSocketTransform(socketName);
    outStart = socketTransform.GetLocation();
    outForwardVector = socketTransform.Rotator().Vector();
}

void APlayerCharacter::spawnFireEffect(FName socketName, FVector& location, FVector& direction)
{
    if (!fireParticle)
        return;
    
    float vectorLength = 10.f;
    FVector adjustedLocation = location + direction * vectorLength;
    UParticleSystemComponent* fireEffect = UGameplayStatics::SpawnEmitterAttached(
        fireParticle,
        weaponComponent,
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

FRotator APlayerCharacter::getAimRotation()
{
    FRotator controlRotation = GetControlRotation();
    FRotator deltaRotation = controlRotation - GetActorRotation();
    deltaRotation.Pitch *= -1;
    
    return FRotator(0, deltaRotation.Yaw,deltaRotation.Pitch);
}

bool APlayerCharacter::performLineTrace(const FVector& start, const FVector& end, FHitResult& outHit) const
{
    FCollisionQueryParams collisionParams;
    return GetWorld()->LineTraceSingleByChannel(
        outHit,
        start,
        end,
        ECC_Visibility,
        collisionParams
    );
}

void APlayerCharacter::handleHit(const FHitResult& hitResult)
{
    // if (hitResult.GetActor())
    // {
    //     UE_LOG(LogTemp, Warning, TEXT("Hit Actor: %s"), *hitResult.GetActor()->GetName());
    // }

    if (impactParticle)
    {
        UParticleSystemComponent* pointEffect = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            impactParticle,
            hitResult.ImpactPoint,
            hitResult.Normal.ToOrientationRotator(),
            true,
            EPSCPoolMethod::AutoRelease
        );

        if (pointEffect)
        {
            float scale = 0.5f;
            pointEffect->SetWorldScale3D(FVector(scale));
        }
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
