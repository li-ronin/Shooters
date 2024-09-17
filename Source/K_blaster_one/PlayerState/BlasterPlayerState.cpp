// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "K_blaster_one/Character/BlasterCharacter.h"
#include "K_blaster_one/PlayerController/BlasterPlayerController.h"

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	Character = (Character==nullptr) ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = (Controller==nullptr) ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	Score += ScoreAmount;
	Character = (Character==nullptr) ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if(Character)
	{
		Controller = (Controller==nullptr) ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if(Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}
}
