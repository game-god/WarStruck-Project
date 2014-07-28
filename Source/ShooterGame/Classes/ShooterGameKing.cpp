// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameKing.h"
#include "CallbackDevice.h"

UShooterGameKing* UShooterGameKing::ShooterKingInstance = nullptr;

#if SHOOTER_CONSOLE_UI	
static EShooterGameState InitialFrontEndState = EShooterGameState::EWelcomeScreen;
#else
static EShooterGameState InitialFrontEndState = EShooterGameState::EMainMenu;
#endif

UShooterGameKing::UShooterGameKing(const class FPostConstructInitializeProperties& PCIP) 
	: Super(PCIP)
{	
	CurrentState = InitialFrontEndState;
}

void UShooterGameKing::Initialize()
{
	check(ShooterKingInstance == nullptr);
	ShooterKingInstance = NewObject<UShooterGameKing>();  
	ShooterKingInstance->InitInternal();		
}

void UShooterGameKing::InitInternal()
{
	// game requires the ability to ID users.
	const auto OnlineSub = IOnlineSubsystem::Get();
	check(OnlineSub);
	const auto IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());		

	// bind any OSS delegates the King needs to handle
	OnLoginChangedDelegate.BindUObject(this, &UShooterGameKing::HandleUserLoginChanged);
	IdentityInterface->AddOnLoginChangedDelegate(OnLoginChangedDelegate);

	FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(this, &UShooterGameKing::HandleWillDeactivate);
	FCoreDelegates::OnSafeFrameChangedEvent.AddUObject(this, &UShooterGameKing::HandleSafeFrameChanged);
}

void UShooterGameKing::HandleUserLoginChanged(int32 GameUserIndex)
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	check(OnlineSub);
	const auto IdentityInterface = OnlineSub->GetIdentityInterface();
	check(IdentityInterface.IsValid());

	// Shootergame doesn't require sophisticated logout handling.  If we are playing a game,
	// just bail if a splitscreen (local) user that is playing the game signs out.
	ELoginStatus::Type UserLoginState = IdentityInterface->GetLoginStatus(GameUserIndex);
	if (CurrentState == EShooterGameState::EPlaying)
	{
		if (GEngine && GEngine->GameViewport)
		{
			// we only want to bail if a splitscreen player logged out.  If a user that wasn't playing logged out, then we don't care.
			UWorld* GameWorld = GEngine->GameViewport->GetWorld();
			for(FConstPlayerControllerIterator Iterator = GameWorld->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{					
				APlayerController* Controller = *Iterator;
				if(Controller && Controller->IsLocalController())
				{
					ULocalPlayer* Player = Cast<ULocalPlayer>(Controller->Player);
					if(Player)
					{
						if (Player->ControllerId == GameUserIndex)
						{
#if SHOOTER_CONSOLE_UI
							// on consoles, if the sign-out is the primary player, we need to go all the way back to the welcome screen
							// or the ownership of the frontend becomes ambiguous.
							if (Controller->IsPrimaryPlayer())
							{
								SetCurrentState(EShooterGameState::EWelcomeScreen);
							}
							else
							{
								SetCurrentState(EShooterGameState::EMainMenu);
							}
#else								
							SetCurrentState(InitialFrontEndState);
#endif
							break;
						}
					}
				}
			}
		}
	}
	else if(CurrentState == EShooterGameState::EMainMenu)
	{
		if (GEngine && GEngine->GameViewport)
		{
			// we only want to bail if the main menu owner logged out.
			UWorld* GameWorld = GEngine->GameViewport->GetWorld();

			ULocalPlayer* LocalPlayer = GameWorld->GetFirstLocalPlayerFromController();
			if(LocalPlayer)
			{
				if (LocalPlayer->ControllerId == GameUserIndex)
				{
					SetCurrentState(InitialFrontEndState);
				}
			}
		}
	}
}

void UShooterGameKing::HandleSafeFrameChanged()
{
	UCanvas::UpdateAllCanvasSafeZoneData();
}

void UShooterGameKing::SetCurrentState(EShooterGameState NewState)
{
	CurrentState = NewState;
}

bool UShooterGameKing::Tick(float DeltaSeconds)
{
	return true;
}

void UShooterGameKing::RemoveSplitScreenPlayers(UWorld * WorldObj)
{
	// if we had been split screen, toss the extra players now
	// @todo: I couldn't find a great way/place to make the main menu not be split screen - so treat this as temporary
	const int MaxSplitScreenPlayers = 4;
	ULocalPlayer* PlayersToRemove[MaxSplitScreenPlayers];
	int RemIndex = 0;

	for(FConstPlayerControllerIterator Iterator = WorldObj->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		// only remove local split screen controllers, so that the later loop still sends good messages to any remote controllers.
		APlayerController* Controller = *Iterator;
		if(Controller && Controller->IsLocalController() && !Controller->IsPrimaryPlayer())
		{
			ULocalPlayer* ExPlayer = Cast<ULocalPlayer>(Controller->Player);
			if(ExPlayer)
			{
				// don't actually remove players here because it affects our iterator.
				PlayersToRemove[RemIndex++]  = ExPlayer;					
			}
		}
	}

	// safe cached remove
	for (int i = 0; i < RemIndex; ++i)
	{
		GEngine->GameViewport->RemovePlayer(PlayersToRemove[i]);
	}	
}

EShooterGameState UShooterGameKing::GetInitialFrontendState() const
{
	return InitialFrontEndState;
}

void UShooterGameKing::HandleWillDeactivate()
{
	if(CurrentState != EShooterGameState::EPlaying)
	{
		return;
	}

	// Just have the first player controller pause the game.
	UWorld* GameWorld = GEngine->GameViewport->GetWorld();

	// protect against a second pause menu loading on top of an existing one if someone presses the Jewel / PS buttons.
	bool bNeedsPause = true;
	for (FConstControllerIterator It = GameWorld->GetControllerIterator(); It; ++It)
	{
		AShooterPlayerController* Controller = Cast<AShooterPlayerController>(*It);		
		if (Controller)
		{
			if (Controller->IsPaused())
			{
				bNeedsPause = false;
				break;
			}
		}
	}

	if (bNeedsPause)
	{	
		AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GameWorld->GetFirstPlayerController());
		if(Controller)
		{
			Controller->ShowInGameMenu();
		}
	}
}

