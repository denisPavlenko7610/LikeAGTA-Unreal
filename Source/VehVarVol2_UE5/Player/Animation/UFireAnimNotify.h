#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "UFireAnimNotify.generated.h"

UCLASS()
class UFireAnimNotify : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& NotifyEventReference) override;

protected:
	void fire(USkeletalMeshComponent* skeletalMesh);
};
