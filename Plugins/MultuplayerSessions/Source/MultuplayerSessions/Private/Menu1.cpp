// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu1.h"

#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"


void UMenu1::MenuSetup(int32 NumberOfPublicConnections ,FString TypeOfMatch,FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"),*LobbyPath);
	NumOfPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	//将此标志设置为 true 将允许此构件在单击或导航到时接受焦点。
	bIsFocusable = true;
	UWorld*World = GetWorld();
	if(World)
	{
		APlayerController * PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}
	UGameInstance* GameInstance = GetGameInstance();
	if(GameInstance)
	{
		MultiplayerSessionsSubsystem=GameInstance ->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this,&ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionComplete.AddUObject(this,&ThisClass::OnFindSession);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this,&ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this,&ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this,&ThisClass::OnStartSession);
	}
}

void UMenu1::HostButtonClicked()
{
	//在点击过后，暂时不能再点击，
	HostButton->SetIsEnabled(false);
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Yellow,
			FString(TEXT("HostButtonClicked!"))
		);
	}
	if(MultiplayerSessionsSubsystem)
	{
		//创建会话后，就立马跳转到关卡。
		MultiplayerSessionsSubsystem->CreateSession(NumOfPublicConnections,MatchType);
		
	}
}

void UMenu1::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Black,
			FString(TEXT("JoinButtonClicked!"))
		);
	}
	if(MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSession(10000);
	}
	
}

bool UMenu1::Initialize()
{
	if(!Super::Initialize())return false;
	if(HostButton)
	{
		HostButton->OnClicked.AddDynamic(this,&ThisClass::HostButtonClicked);
	}
	if(JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this,&ThisClass::JoinButtonClicked);
	}
	return true;
}
//解出关闭鼠标输入的问题
void UMenu1::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if(World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if(PlayerController)
		{
			FInputModeGameOnly InputModeGameData;
			PlayerController->SetInputMode(InputModeGameData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu1::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu1::OnCreateSession(bool bWasSuccessful)
{
	if(bWasSuccessful)
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Black,
				FString(TEXT("Session Created Successfully!"))
			);
		}
		UWorld* World = GetWorld();
		if(World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Create Session Failed!"))
			);
		}
		//等建立好了session，且建立失败了，就放开按钮再次点击
		HostButton->SetIsEnabled(true
			);
	}
	
}

void UMenu1::OnFindSession(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
	for (auto Result : SearchResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)
		{
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	if(!bWasSuccessful||SearchResults.Num()==0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu1::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if(Result!=EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu1::OnDestroySession(bool bWasSuccessful)
{
	
}

void UMenu1::OnStartSession(bool bWasSuccessful)
{
	
}
