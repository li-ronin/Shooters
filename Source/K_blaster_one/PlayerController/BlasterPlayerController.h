
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class K_BLASTER_ONE_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float ScoreAmount);
	void SetHUDDefeat(int32 DefeatAmount);
	void SetHUDWeaponAmmo(int32 DefeatAmount);
	void SetHUDCarriedAmmo(int32 CarriedAmmo);
	void OnPossess(APawn* aPawn) override;
protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
};
