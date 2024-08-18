// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenuWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumPublicConnections=4, FString MatchType=FString(TEXT("FreeForAll")), FString Path = FString(TEXT("/Game/ThirdPerson/Maps/Lobby")));
protected:
	bool Initialize() override;
	void NativeDestruct() override;

	////
	///接收Multiplayer Session Subsystem消息的回调函数
	///
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
private:
	//// Widget中的按钮
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;	// 注意蓝图中的widget名字要和C++中的变量名称相同

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	//// 处理按钮函数
	UFUNCTION()
	void OnHostButtonClicked();
	
	UFUNCTION()
	void OnJoinButtonClicked();
	
	////

	void MenuTearDown();	// 移除菜单Widget
	
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;// 处理所有Online Session功能的子系统

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLobby{TEXT("")};
};
