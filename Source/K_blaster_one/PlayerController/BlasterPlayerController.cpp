

#include "BlasterPlayerController.h"

#include "K_blaster_one/HUD/BlasterHUD.h"
#include "K_blaster_one/HUD/CharacterOverlay.h"
#include "K_blaster_one/Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;
	if(bHUDVaild)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::SetHUDScore(float ScoreAmount)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;
	if(bHUDVaild)
	{
		FString ScoreAmountText = FString::Printf(TEXT("%d"), FMath::FloorToInt(ScoreAmount));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreAmountText));
	}
}

void ABlasterPlayerController::SetHUDDefeat(int32 DefeatAmount)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatAmount;
	if(bHUDVaild)
	{
		FString DefeatAmountText = FString::Printf(TEXT("%d"), DefeatAmount);
		BlasterHUD->CharacterOverlay->DefeatAmount->SetText(FText::FromString(DefeatAmountText));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if(bHUDVaild)
	{
		FString AmmoAmountText = FString::Printf(TEXT("%d/50"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoAmountText));
	}
}

void ABlasterPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(aPawn);
	if(BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}
