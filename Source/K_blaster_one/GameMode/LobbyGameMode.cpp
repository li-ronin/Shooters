// 检查有多少玩家已经连接到Lobby关卡中，达到一定数量则创建游戏地图

#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if(GameState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		if(NumberOfPlayers >= 2)
		{
			UWorld* World = GetWorld();
			if(World)
			{
				// GameMode仅存在于服务器上，所以GameMode类中是服务器里面
				// 设置GameMode类中的无缝传送
				bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));	
			}
			
		}
		// APlayerState* NewPlayerState = NewPlayer->GetPlayerState<APlayerState>();
		// FString PlayerName = NewPlayerState->GetPlayerName();
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}
