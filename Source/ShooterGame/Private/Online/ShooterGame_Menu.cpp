// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterMainMenu.h"
#include "ShooterWelcomeMenu.h"
#include "ShooterMessageMenu.h"
#include "ShooterGameLoadingScreen.h"

AShooterGame_Menu::AShooterGame_Menu(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	PlayerControllerClass = AShooterPlayerController_Menu::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	SetTickableWhenPaused(true);

	CurrentState = EShooterGameState::EUnknown;
}

void AShooterGame_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AShooterGame_Menu::GetGameSessionClass() const
{
	return AShooterGameSession::StaticClass();
}

// Hacky but for the moment assume if this menu is ending, then we must want to start playing.  Fixes issues with clients using the 'open' command.
// State management will change after next week's design meeting.
void AShooterGame_Menu::HandleLeavingMap()
{
	ClearWelcomeMenu();
	ClearMainMenu();
	ClearMessageMenu();
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EPlaying);
}

/** Starts the game */
bool AShooterGame_Menu::HostGame(APlayerController* PCOwner, const FString& GameType, const FString& InTravelURL)
{
	bool bResult = false;

	check(PCOwner != NULL);
	if (PCOwner)
	{
		AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
		if (Session)
		{
			ULocalPlayer * LP = Cast<ULocalPlayer>(PCOwner->Player);
			if (LP != NULL)
			{
				TravelURL = InTravelURL;
				bool bIsLanMatch = TravelURL.Contains(TEXT("?bIsLanMatch"));
				Session->OnCreatePresenceSessionComplete().AddUObject(this, &AShooterGame_Menu::OnCreatePresenceSessionComplete);
				if (Session->HostSession(LP->ControllerId, GameSessionName, GameType, bIsLanMatch, true, AShooterGameSession::DEFAULT_NUM_PLAYERS))
				{
					BeginSession();
					bResult = true;
				}
			}
		}
	}

	return bResult;
}

/** Callback which is intended to be called upon session creation */
void AShooterGame_Menu::OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful)
{
	AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
	if (Session)
	{
		Session->OnCreatePresenceSessionComplete().RemoveUObject(this, &AShooterGame_Menu::OnCreatePresenceSessionComplete);
		if (bWasSuccessful)
		{
			// Travel to the specified match URL
			GetWorld()->ServerTravel(TravelURL);
		}
		else
		{
			FString ReturnReason = NSLOCTEXT("NetworkErrors", "CreateSessionFailed", "Failed to create session.").ToString();
			FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
			ShowMessageThenGoMain(ReturnReason, OKButton, FString());
		}
	}
}

/** Callback which is intended to be called upon session creation */
void AShooterGame_Menu::OnCreateGameSessionComplete(FName SessionName, bool bWasSuccessful)
{
	AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
	if (Session)
	{
		Session->OnCreatePresenceSessionComplete().RemoveUObject(this, &AShooterGame_Menu::OnCreatePresenceSessionComplete);
	}
}

/** Initiates the session searching */
bool AShooterGame_Menu::FindSessions(APlayerController* PCOwner, bool bFindLAN)
{
	bool bResult = false;

	check(PCOwner != NULL);
	if (PCOwner)
	{
		AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
		if (Session)
		{
			ULocalPlayer * LP = Cast<ULocalPlayer>(PCOwner->Player);
			if (LP)
			{
				Session->OnFindSessionsComplete().RemoveAll(this);

				Session->OnFindSessionsComplete().AddUObject(this, &AShooterGame_Menu::OnSearchSessionsComplete);
				Session->FindSessions(LP->ControllerId, GameSessionName, bFindLAN, true);

				bResult = true;
			}
		}
	}

	return bResult;
}

/** Callback which is intended to be called upon finding sessions */
void AShooterGame_Menu::OnSearchSessionsComplete(bool bWasSuccessful)
{
	AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
	if (Session)
	{
		Session->OnFindSessionsComplete().RemoveUObject(this, &AShooterGame_Menu::OnSearchSessionsComplete);
	}
}

/** Joins one of sessions previously found */
bool AShooterGame_Menu::JoinSession(APlayerController* PCOwner, int32 SessionIndexInSearchResults)
{
	bool bResult = false;

	check(PCOwner != NULL);
	if (PCOwner)
	{
		AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
		if (Session)
		{
			ULocalPlayer * LP = Cast<ULocalPlayer>(PCOwner->Player);
			if (LP)
			{
				JoiningControllerId = LP->ControllerId;
				
				AddFailureHandlers();

				Session->OnJoinSessionComplete().AddUObject(this, &AShooterGame_Menu::OnJoinSessionComplete);
				bResult = Session->JoinSession(JoiningControllerId, GameSessionName, SessionIndexInSearchResults);

				BeginSession();
				bResult = true;
			}
		}
	}

	return bResult;
}

/** Callback which is intended to be called upon finding sessions */
void AShooterGame_Menu::OnJoinSessionComplete(bool bWasSuccessful)
{
	AShooterGameSession* Session = Cast<AShooterGameSession>(GameSession);
	if (Session)
	{
		Session->OnJoinSessionComplete().RemoveUObject(this, &AShooterGame_Menu::OnJoinSessionComplete);
	}

	if (bWasSuccessful)
	{
		// travel to session
		IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			FString URL;
			IOnlineSessionPtr Sessions = OnlineSub->GetSessionInterface();
			if (Sessions.IsValid() && Sessions->GetResolvedConnectString(GameSessionName, URL))
			{
				APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), JoiningControllerId);
				if (PC)
				{
					PC->ClientTravel(URL, TRAVEL_Absolute);
				}
			}
			else
			{
				FString FailReason = NSLOCTEXT("NetworkErrors", "TravelSessionFailed", "Travel to Session failed.").ToString();
				FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
				ShowMessageThenGoMain(FailReason, OKButton, FString());
				UE_LOG(LogOnlineGame, Warning, TEXT("Failed to travel to session upon joining it"));
			}
		}
	}
	else
	{
		APlayerController* const PC = UGameplayStatics::GetPlayerController(GetWorld(), JoiningControllerId);
		if (PC)
		{
			FString ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.").ToString();
			FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
			ShowMessageThenGoMain(ReturnReason, OKButton, FString());
		}
	}
}

void AShooterGame_Menu::BeginPlay()
{
	// hack to catch all cases coming back from play to make sure we don't end up in blank UI.
	// when Framework team moves King to a push model, this can go away.
	UShooterGameKing& ShooterKing = UShooterGameKing::Get();

	// if we have a net game pending, consider ourselves already playing
	FWorldContext * WorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
	EShooterGameState GameStateAtMenuBegin = (WorldContext && WorldContext->PendingNetGame) ? EShooterGameState::EPlaying : ShooterKing.GetInitialFrontendState();
	ShooterKing.SetCurrentState(GameStateAtMenuBegin);

	Super::BeginPlay();
	ConformToKingState();
}

void AShooterGame_Menu::ToMainMenu()
{
	ClearWelcomeMenu();
	ClearMessageMenu();

	// player 0 gets to own the UI
	APlayerController* const FirstPC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	ShooterMainMenuUI = MakeShareable(new FShooterMainMenu());
	ShooterMainMenuUI->Construct(FirstPC, this);
	ShooterMainMenuUI->AddMenuToGameViewport();

	// Set rich presence.
	const auto Presence = Online::GetPresenceInterface();
	if (Presence.IsValid() && FirstPC && FirstPC->PlayerState && FirstPC->PlayerState->UniqueId.IsValid())
	{
		FPresenceProperties Props;
		Props.Add(DefaultPresenceKey, FVariantData(FString("OnMenu")));
		Presence->SetPresence(*FirstPC->PlayerState->UniqueId, Props);
	}

	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EMainMenu);
	// Remove the local session/travel failure bindings if they exist
	if (GEngine->OnNetworkFailure().IsBoundToObject(this) == true)
	{
		GEngine->OnNetworkFailure().RemoveUObject(this, &AShooterGame_Menu::JoinLocalSessionFailure);
	}
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == true)
	{
		GEngine->OnTravelFailure().RemoveUObject(this, &AShooterGame_Menu::TravelLocalSessionFailure);
	}
}

void AShooterGame_Menu::ClearWelcomeMenu()
{
	if (ShooterWelcomeMenuUI.IsValid())
	{	
		ShooterWelcomeMenuUI->RemoveFromGameViewport();
		ShooterWelcomeMenuUI = nullptr;
	}
}

void AShooterGame_Menu::ClearMainMenu()
{
	if (ShooterMainMenuUI.IsValid())
	{		
		ShooterMainMenuUI->RemoveMenuFromGameViewport();
		ShooterMainMenuUI = nullptr;
	}	
}

void AShooterGame_Menu::ClearMessageMenu()
{
	if (MessageMenuUI.IsValid())
	{
		MessageMenuUI->RemoveFromGameViewport();
		MessageMenuUI = nullptr;
	}
}

void AShooterGame_Menu::TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ReasonString)
{
	ShowJoinFailureMessageAndGoMain(ReasonString);
}

void AShooterGame_Menu::JoinLocalSessionFailure(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ReasonString)
{
	ShowJoinFailureMessageAndGoMain(ReasonString);
}

void AShooterGame_Menu::ToWelcome()
{
	ClearMainMenu();
	ClearMessageMenu();
	ULocalPlayer* LocalPlayer = GetWorld()? GetWorld()->GetFirstLocalPlayerFromController(): nullptr;	
	
	check(!ShooterWelcomeMenuUI.IsValid());
	ShooterWelcomeMenuUI = MakeShareable(new FShooterWelcomeMenu);
	ShooterWelcomeMenuUI->Construct(LocalPlayer, this);
	ShooterWelcomeMenuUI->AddToGameViewport();	
}

void AShooterGame_Menu::ConformToKingState()
{
	UShooterGameKing& GameKing = UShooterGameKing::Get();
	EShooterGameState CurrentKingState = GameKing.GetCurrentState();
	if (CurrentKingState != CurrentState)
	{
		CurrentState = CurrentKingState;
		switch (CurrentKingState)
		{
			case EShooterGameState::EMainMenu:
				ToMainMenu();
				break;
			case EShooterGameState::EWelcomeScreen:
				ToWelcome();
				break;
			case EShooterGameState::EMessageScreen:
				break;
			case EShooterGameState::EPlaying:				
				UE_LOG(LogShooter, Log, TEXT("Playing state in AShooterGame_Menu, mode should be ending"));
				break;
			default:
				UE_LOG(LogShooter, Warning, TEXT("Unhandled KingState: %i in AShooterGame_Menu"), (int)CurrentKingState);
				break;
		}
	}
}

void AShooterGame_Menu::Tick(float DeltaSeconds)
{
	ConformToKingState();
}

void AShooterGame_Menu::ShowMessageThenGoMain(const FString& Message, const FString& OKButtonString, const FString& CancelButtonString)
{
	ClearMainMenu();
	ClearWelcomeMenu();

	ULocalPlayer* LocalPlayer = GetWorld() ? GetWorld()->GetFirstLocalPlayerFromController() : nullptr;

	check(!MessageMenuUI.IsValid());
	MessageMenuUI = MakeShareable(new FShooterMessageMenu);
	MessageMenuUI->Construct(LocalPlayer, this, Message, OKButtonString, CancelButtonString);
	MessageMenuUI->AddToGameViewport();

	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EMessageScreen);
}

void AShooterGame_Menu::BeginSession()
{
	ClearWelcomeMenu();
	ClearMainMenu();

	ShowLoadingScreen();
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EPlaying);
}

void AShooterGame_Menu::ShowLoadingScreen()
{	
	IShooterGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IShooterGameLoadingScreenModule>("ShooterGameLoadingScreen");
	if (LoadingScreenModule != NULL)
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

void AShooterGame_Menu::ShowJoinFailureMessageAndGoMain(const FString& InFailureReason)
{
	AShooterPlayerController_Menu* const FirstPC = Cast<AShooterPlayerController_Menu>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (FirstPC != NULL)
	{
		FString ReturnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join Session failed.").ToString();
		if (InFailureReason.IsEmpty() == false)
		{
			ReturnReason += " ";
			ReturnReason += InFailureReason;
		}

		FString OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK").ToString();
		ShowMessageThenGoMain(ReturnReason, OKButton, FString());
	}
}

void AShooterGame_Menu::AddFailureHandlers()
{
	// Add network/travel error handlers (if they are not already there)
	if (GEngine->OnNetworkFailure().IsBoundToObject(this) == false)
	{
		GEngine->OnNetworkFailure().AddUObject(this, &AShooterGame_Menu::JoinLocalSessionFailure);
	}
	if (GEngine->OnTravelFailure().IsBoundToObject(this) == false)
	{
		GEngine->OnTravelFailure().AddUObject(this, &AShooterGame_Menu::TravelLocalSessionFailure);
	}
}

