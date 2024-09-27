
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
	void SetHUDMatchCountDown(float CountDownTime);
	void SetHUDWarmupCountDown(float CountDownTime);
	void OnPossess(APawn* aPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float GetServerTime();	// syncd with server world clock
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCoolDown();
protected:
	virtual void BeginPlay() override;
	void SetHUDTime();
	void PollInit();
	/**
	 * Sync time between client and server
	 */
	// Request current server time, 
	UFUNCTION(Server, Reliable)
	void Server_RequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client
	UFUNCTION(Client, Reliable)
	void Client_ReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	float ClientServerDeltaTime = 0.f;

	UPROPERTY(EditAnywhere, Category = Sync)
	float SyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void CheckTimeSync(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void Server_CheckMatchState();

	UFUNCTION(Client, Reliable)
	void Client_JoinMidGame(FName StateOfMatch, float Warmup, float Match, float StartingTime);
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
	
	float LevelStartingTime = 0.f;
	float WarmupTime = 0.f;
	float MatchTime = 0.f;
	uint32 CountDownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
	
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDDefeat;
};
