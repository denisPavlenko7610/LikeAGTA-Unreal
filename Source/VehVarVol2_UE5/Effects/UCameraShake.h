#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraShakeBase.h"
#include "LegacyCameraShake.h"
#include "UCameraShake.generated.h"

UCLASS(Blueprintable, BlueprintType)
class UCameraShake : public ULegacyCameraShake
{
	GENERATED_BODY()

public:
	UCameraShake();
};
