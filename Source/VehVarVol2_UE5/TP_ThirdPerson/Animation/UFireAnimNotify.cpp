#include "UFireAnimNotify.h"

#include "Components/SkeletalMeshComponent.h"
#include "VehVarVol2_UE5/TP_ThirdPerson/APlayerCharacter.h"

void UFireAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& NotifyEventReference)
{
	Super::Notify(MeshComp, Animation, NotifyEventReference);
	
	fire(MeshComp);
}

void UFireAnimNotify::fire(USkeletalMeshComponent* skeletalMesh)
{
	if (skeletalMesh->GetOwner() == nullptr)
		return;
	
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(skeletalMesh->GetOwner()))
	{
		PlayerCharacter->fire();
	}
}
