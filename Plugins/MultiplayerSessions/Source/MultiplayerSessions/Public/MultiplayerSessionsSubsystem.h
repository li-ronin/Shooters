// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

////
// 为MenuWidget类声明我们的自定义Delegate，以供MenuWidget回调
////EOnJoinSessionCompleteResult和FOnlineSessionSearchResult与蓝图不兼容（与U结构反射系统不兼容的类型），所以Find和Join使用多播代理而不是Dynamic多播代理
///
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionComplete, const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);	
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();
	////
	// 处理session的函数，The Menu class will call these
	///
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	void DestroySession();
	void StartSession();
	
	// 为MenuWidget类绑定回调函数的自定义Delegate
	FMultiplayerOnCreateSessionComplete MultiplayerCreateSessionComplete;
	FMultiplayerOnFindSessionComplete MultiplayerFindSessionComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerStartSessionComplete;
protected:
	////
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	
	
private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;
	//// Delegate
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	//// DelegateHandle
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{false}; // 在创建会话时若有已存在一个会话了为true，用来确保旧会话已销毁才可以创建新的。
	// 存储当创建会话失败时的传入的创建信息 CreateSession(int32 NumPublicConnections, FString MatchType)
	int32 LastNumPublicConnections;
	FString LastMatchType;
	
};
