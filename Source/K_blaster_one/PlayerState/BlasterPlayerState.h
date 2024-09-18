// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class K_BLASTER_ONE_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	void OnRep_Score() override;
	UFUNCTION()
	void OnRep_Defeat();
	void AddToScore(float ScoreAmount);
	void AddToDefeat(int32 DefeatAmount);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// 在该函数中注册要复制的变量
private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	
	UPROPERTY(ReplicatedUsing = OnRep_Defeat)
	int32 Defeats;
};
