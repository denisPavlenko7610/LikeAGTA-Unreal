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
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/HealthComponent.h"
#include "Components/WeaponComponent.h"
#include "Interactions/VehicleInteraction.h"
#include "VehVarVol2_UE5/UI/UHudWidget.h"

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

    WeaponComponent = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
    WeaponComponent->SetupAttachment(GetMesh(), FName("WeaponSocket"));
    WeaponComponent->SetHiddenInGame(true);

    _initialArmLength = 300.0f;
    _targetArmLength = 150.0f;
    _aimLerpDurationS = 0.25f;
    _elapsedTimeS = 0.0f;
}

void APlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    createHUD();
    
    _vehicleInteraction = NewObject<UVehicleInteraction>(this);
    _vehicleInteraction->init(this);

    WeaponComponent->init(this);
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
        EnhancedInputComponent->BindAction(getWeaponAction, ETriggerEvent::Started, WeaponComponent, &UWeaponComponent::toggleWeapon);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, WeaponComponent, &UWeaponComponent::fireAnimation);
        EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, WeaponComponent, &UWeaponComponent::stopFire);
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ThisClass::aim);
        EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::stopAim);
    }
    else
    {
        UE_LOG(LogTemplateCharacter, Error,
            TEXT("'%s' Failed to find an Enhanced Input component! "
                 "This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."),
            *GetNameSafe(this));
    }
}

void APlayerCharacter::createHUD()
{
    if (hudWidget == nullptr)
        return;
    
    _hud = CreateWidget<UHudWidget>(GetWorld(), hudWidget);
    _hud->AddToViewport();
    _hud->hideCrosshair();
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

void APlayerCharacter::aim(const FInputActionValue& Value)
{
    if (!WeaponComponent->rifleEquipped)
        return;

    _hud->showCrosshair();
    _initialArmLength = CameraBoom->TargetArmLength;
    _rightOffset = FVector2d(15,50);
    _targetArmLength = 150.0f;
    _elapsedTimeS = 0.0f;
    IsAiming = true;
    
    GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
}

void APlayerCharacter::stopAim(const FInputActionValue& Value)
{
    if (!WeaponComponent->rifleEquipped)
        return;

    _hud->hideCrosshair();
    _initialArmLength = CameraBoom->TargetArmLength;
    _rightOffset = FVector2d(0,0);
    _targetArmLength = 300.0f;
    _elapsedTimeS = 0.0f;
    IsAiming = false;
    
    GetWorld()->GetTimerManager().SetTimer(LerpTimerHandle, this, &APlayerCharacter::updateAimLerp, GetWorld()->GetDeltaSeconds(), true);
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
        if (ACar* Car = Cast<ACar>(Overlap.GetActor()))
        {
            _vehicleInteraction->enterVehicle(Car);
            return;
        }
    }
}

void APlayerCharacter::getSocketTransformAndVectors(const FName& socketName, FVector& outStart, FVector& outForwardVector) const
{
    FTransform socketTransform = WeaponComponent->GetSocketTransform(socketName);
    outStart = socketTransform.GetLocation();
    outForwardVector = socketTransform.Rotator().Vector();
}

FRotator APlayerCharacter::getAimRotation()
{
    FRotator controlRotation = GetControlRotation();
    FRotator deltaRotation = controlRotation - GetActorRotation();
    deltaRotation.Pitch *= -1;
    
    return FRotator(0, deltaRotation.Yaw,deltaRotation.Pitch);
}

void APlayerCharacter::onHealthChanged(UHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType,
    AController* InstigatedBy, AActor* DamageCauser)
{
    Super::onHealthChanged(OwningHealthComp, Health, HealthDelta, DamageType, InstigatedBy, DamageCauser);
    
    _hud->updateHealthBar(Health, OwningHealthComp->getMaxHealth());
}


