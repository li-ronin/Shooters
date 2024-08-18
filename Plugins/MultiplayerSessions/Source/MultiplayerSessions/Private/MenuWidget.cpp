// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuWidget.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"


void UMenuWidget::MenuSetup(int32 NumPublicConne, FString TypeofMatch, FString Path)
{
    PathToLobby = FString::Printf(TEXT("%s?listen"), *Path);
    
    NumPublicConnections = NumPublicConne;
    MatchType = TypeofMatch;
    // 与Widget特定功能相关的函数
    AddToViewport();
    SetVisibility(ESlateVisibility::Visible);
    bIsFocusable = true;
    
    //
    UWorld* World = GetWorld();
    if(World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if(PlayerController)
        {
            FInputModeUIOnly InputModeData;
            InputModeData.SetWidgetToFocus(TakeWidget());
            InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 是否锁定鼠标到视口
            PlayerController->SetInputMode(InputModeData);
            PlayerController->SetShowMouseCursor(true);
        }
    }
    UGameInstance* GameInstance = GetGameInstance();
    if(GameInstance)
    {
        MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
        //绑定回调函数
        MultiplayerSessionsSubsystem->MultiplayerCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
        MultiplayerSessionsSubsystem->MultiplayerFindSessionComplete.AddUObject(this, &ThisClass::OnFindSession);
        MultiplayerSessionsSubsystem->MultiplayerJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
        MultiplayerSessionsSubsystem->MultiplayerDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
        MultiplayerSessionsSubsystem->MultiplayerStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
    }
   
}

bool UMenuWidget::Initialize()
{
    if(!Super::Initialize())
    {
        return false; 
    }
    if(HostButton)
    {
        HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);    
    }
    if(JoinButton)
    {
        JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
    }
    return true;
}

void UMenuWidget::NativeDestruct()
{
    MenuTearDown();
    Super::NativeDestruct();
}

void UMenuWidget::OnCreateSession(bool bWasSuccessful)
{
    if(!bWasSuccessful)
    {
        HostButton->SetIsEnabled(true); // 创建失败, 恢复Host按钮
        if(GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                15.f,
                FColor::Red,
                FString::Printf(TEXT("Menu Failed to create Session !!")));
        }
        return;
    }

    // 创建完Session后进入Lobby
    UWorld* World = GetWorld();
    if(World)
    {
        World->ServerTravel(PathToLobby);
    }
}

void UMenuWidget::OnFindSession(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
    if(!MultiplayerSessionsSubsystem)return;
 
    for(auto Result: SessionResults)
    {
        FString SettingsValue;
        Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
        if(SettingsValue == MatchType)
        {
            MultiplayerSessionsSubsystem->JoinSession(Result);
            return;
        }
    }
    if(!bWasSuccessful || SessionResults.Num()==0)
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenuWidget::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
    IOnlineSubsystem* OnlineSystem = IOnlineSubsystem::Get();
    if(OnlineSystem)
    {
        IOnlineSessionPtr SessionInterface = OnlineSystem->GetSessionInterface();
        if(SessionInterface.IsValid())
        {
            FString Address;
            SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
            APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
            if(PlayerController)
            {
                PlayerController->ClientTravel(Address, TRAVEL_Absolute);
            }
        }
    }
    //  恢复Join按钮，这里加恢复是为了防止有人在不销毁会话的情况下退出了会话，在DestroySession被调用之前Session的地址是无效的
    if(Result != EOnJoinSessionCompleteResult::Success)
    {
        JoinButton->SetIsEnabled(true);
    }
}

void UMenuWidget::OnDestroySession(bool bWasSuccessful)
{
}

void UMenuWidget::OnStartSession(bool bWasSuccessful)
{
}

void UMenuWidget::OnHostButtonClicked()
{
    if(!MultiplayerSessionsSubsystem)return;
    HostButton->SetIsEnabled(false);    // 点击完禁用按钮，防止重复点击
    // 通过对象调用MultplayerSessionSubsystem插件中的CreateSession函数
    MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
}

void UMenuWidget::OnJoinButtonClicked()
{
    if(!MultiplayerSessionsSubsystem)return;
    JoinButton->SetIsEnabled(false);    // 点击完禁用按钮，防止重复点击
    MultiplayerSessionsSubsystem->FindSessions(10000);
}

void UMenuWidget::MenuTearDown()
{
    RemoveFromParent(); // 移除Widget
    UWorld* World = GetWorld();
    if(World)
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if(PlayerController)
        {
            FInputModeGameOnly InputModeData;     // 注意这里是ModeGame不是UI，让我专注于游戏
            PlayerController->SetInputMode(InputModeData);  // 保持默认值InputModeData传递。这样就可以添加输入并移动角色,
            PlayerController->SetShowMouseCursor(false);    // 隐藏光标
        }
    }
}
