// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;
/**
 * 
 */
UCLASS()
class K_BLASTER_ONE_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// 在该函数中注册要复制的变量

	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
