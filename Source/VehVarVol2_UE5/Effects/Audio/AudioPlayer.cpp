
#include "AudioPlayer.h"

#include "Kismet/GameplayStatics.h"
#include "MetasoundSource.h"

void UAudioPlayer::PlayMetaSoundAtLocation(UWorld* world, FVector soundLocation, FName& audioName)
{
	if (!world || audioName.IsNone())
	{
		return;
	}

	if (UMetaSoundSource* MetaSoundToPlay = LoadObject<UMetaSoundSource>(nullptr, *audioName.ToString()))
	{
		UGameplayStatics::SpawnSoundAtLocation(world, MetaSoundToPlay, soundLocation);
	}
}
