#pragma once

#include "CoreMinimal.h"
#include "AudioList.h"
#include "UObject/Object.h"
#include "AudioPlayer.generated.h"


UCLASS()
class VEHVARVOL2_UE5_API UAudioPlayer : public UObject {
	GENERATED_BODY()

public:
	static void PlayMetaSoundAtLocation(UWorld* world, FVector soundLocation, FName& audioName);
};
