// 

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UHudWidget.generated.h"

/**
 * 
 */

class UImage;
class UProgressBar;

UCLASS()
class VEHVARVOL2_UE5_API UHudWidget : public UUserWidget {
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UImage* CrossHairImage;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

protected:
	virtual void NativeConstruct() override;

public:
	void showCrosshair();
	void hideCrosshair();

	void updateHealthBar(float currentHealth, float maxHealth);
};
