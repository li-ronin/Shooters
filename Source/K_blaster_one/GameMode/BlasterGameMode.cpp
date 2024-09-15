// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "K_blaster_one/PlayerController/BlasterPlayerController.h"
#include "K_blaster_one/Character/BlasterCharacter.h"

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter,
	ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if(EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
	
}
