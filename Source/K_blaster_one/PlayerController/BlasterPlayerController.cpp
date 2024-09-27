

#include "BlasterPlayerController.h"

#include "K_blaster_one/HUD/BlasterHUD.h"
#include "K_blaster_one/HUD/CharacterOverlay.h"
#include "K_blaster_one/Character/BlasterCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "K_blaster_one/GameMode/BlasterGameMode.h"
#include "K_blaster_one/HUD/Announcement.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	Server_CheckMatchState();
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetHUDTime();
	CheckTimeSync(DeltaSeconds);
	PollInit();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime >= SyncFrequency)
	{
		TimeSyncRunningTime = 0.f;
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::Server_CheckMatchState_Implementation()
{
	ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if(BlasterGameMode)
	{
		MatchTime = BlasterGameMode->MatchTime;
		WarmupTime = BlasterGameMode->WarmupTime;
		LevelStartingTime = BlasterGameMode->LevelStartingTime;
		MatchState = BlasterGameMode->GetMatchState();
		Client_JoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime);
		// if(BlasterHUD && MatchState==MatchState::WaitingToStart)
		// {
		// 	BlasterHUD->AddAnnouncement();
		// }
	}
}

void ABlasterPlayerController::Client_JoinMidGame_Implementation(FName StateOfMatch, float Warmup, float Match, float StartingTime)
{
	MatchTime = Match;
	WarmupTime = Warmup;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if(BlasterHUD && MatchState==MatchState::WaitingToStart)	// 中途加入已经开始的游戏不会显示Announcement界面UI
	{
		BlasterHUD->AddAnnouncement();
	}
}

float ABlasterPlayerController::GetServerTime()
{
	if(HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();	
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDeltaTime;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		Server_RequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::Server_RequestServerTime_Implementation(float TimeOfClientRequest)
{
	// 客户端向服务器请求当前的服务器时间是多少。该函数仅在服务器上执行
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	Client_ReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::Client_ReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	// 服务器回复客户端请求当前的服务器时间是多少。该函数仅在客户端上执行
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime*0.5f);
	ClientServerDeltaTime = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::PollInit()
{
	if(CharacterOverlay == nullptr)
	{
		if(BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if(CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeat(HUDDefeat);
			}
		}
	}
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
	}else
	{
		bInitCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
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
	}else
	{
		bInitCharacterOverlay = true;
		HUDScore = ScoreAmount;
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
	}else
	{
		bInitCharacterOverlay = true;
		HUDDefeat = DefeatAmount;
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
		FString AmmoAmountText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoAmountText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 CarriedAmmo)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponCarriedAmount;
	if(bHUDVaild)
	{
		FString Carried = FString::Printf(TEXT("%d"), CarriedAmmo);
		BlasterHUD->CharacterOverlay->WeaponCarriedAmount->SetText(FText::FromString(Carried));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if(MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}else if(MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	uint32 SecondLeft = FMath::CeilToInt(TimeLeft);
	if(SecondLeft!=CountDownInt)
	{
		if(MatchState == MatchState::WaitingToStart)
		{
			SetHUDWarmupCountDown(TimeLeft);
		}
		if(MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountDown(TimeLeft);
		} 
	}
	CountDownInt = SecondLeft;
}

void ABlasterPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountDownText;
	if(bHUDVaild)
	{
		int32 Minutes = FMath::FloorToInt(CountDownTime/60.f);
		int32 Seconds = CountDownTime - Minutes*60; 
		FString Time = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(Time));
	}
}

void ABlasterPlayerController::SetHUDWarmupCountDown(float CountDownTime)
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	
	bool bHUDVaild = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;
	if(bHUDVaild)
	{
		int32 Minutes = FMath::FloorToInt(CountDownTime/60.f);
		int32 Seconds = CountDownTime - Minutes*60; 
		FString Time = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(Time));
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

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	if(MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if(MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	if(MatchState == MatchState::CoolDown)
	{
		HandleCoolDown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->AddCharacterOverlay();
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCoolDown()
{
	BlasterHUD = (BlasterHUD==nullptr) ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if(BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if(BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
