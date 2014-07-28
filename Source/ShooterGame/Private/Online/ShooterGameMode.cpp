// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameKing.h"
#include "ShooterSpectatorPawn.h"

AShooterGameMode::AShooterGameMode(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Blueprints/Pawns/PlayerPawn"));
	DefaultPawnClass = PlayerPawnOb.Class;
	
	static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/BotPawn"));
	BotPawnClass = BotPawnOb.Class;

	HUDClass = AShooterHUD::StaticClass();
	PlayerControllerClass = AShooterPlayerController::StaticClass();
	PlayerStateClass = AShooterPlayerState::StaticClass();
	SpectatorClass = AShooterSpectatorPawn::StaticClass();
	GameStateClass = AShooterGameState::StaticClass();

	MinRespawnDelay = 5.0f;

	bAllowBots = true;	

	// need to tick when paused to check king state.
	SetTickableWhenPaused(true);	
}

FString AShooterGameMode::GetBotsCountOptionName()
{
	return FString(TEXT("Bots"));
}

void AShooterGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	// for the moment, if this game mode is active, then we'll just consider ourselves in the playing state.
	// Fixes PIE and dedicated server.  State management will be changing after a design meeting nexxt week.
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EPlaying);	

	CurrentState = EShooterGameState::EPlaying;
	const int32 BotsCountOptionValue = GetIntOption(Options, GetBotsCountOptionName(), 0);
	SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);

	Super::InitGame(MapName, Options, ErrorMessage);
}

void AShooterGameMode::SetAllowBots(bool bInAllowBots, int32 InMaxBots)
{
	bAllowBots = bInAllowBots;
	MaxBots = InMaxBots;
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AShooterGameMode::GetGameSessionClass() const
{
	return AShooterGameSession::StaticClass();
}

void AShooterGameMode::DefaultTimer()
{
	Super::DefaultTimer();

	// don't update timers for Play In Editor mode, it's not real match
	if (GetWorld()->IsPlayInEditor())
	{
		// start match if necessary.
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
		return;
	}

	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;
		
		if (MyGameState->RemainingTime <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				StartMatch();
			}
		}
	}
}

void AShooterGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);

	MyGameState->RemainingTime = RoundTime;
	if (bAllowBots)
	{
		SpawnBotsForGame();
	}

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}

	// probably needs to be done somewhere else when shootergame goes multiplayer
	TriggerRoundStartForLocalPlayers();
}

void AShooterGameMode::FinishMatch()
{
	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();		

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AShooterPlayerState* PlayerState = Cast<AShooterPlayerState>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// probably needs to be done somewhere else when shootergame goes multiplayer
		TriggerRoundEndForLocalPlayers();

		// lock all pawns
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

void AShooterGameMode::TriggerRoundStartForLocalPlayers()
{
	// Send start match event, this will set the CurrentMap stat.
	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	if(Events.IsValid() && Identity.IsValid())
	{	
		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AShooterPlayerController* PC = Cast<AShooterPlayerController>(*It);
			ULocalPlayer* LocalPlayer = PC ? Cast<ULocalPlayer>(PC->Player) : nullptr;
			if (LocalPlayer)
			{					
				int32 UserIndex = LocalPlayer->ControllerId;
				if (UserIndex != -1)
				{
					FOnlineEventParms Params;
					Params.Add( TEXT( "GameplayModeId" ), FVariantData( (int32)1 ) );
					Params.Add( TEXT( "DifficultyLevelId" ), FVariantData( (int32)0 ) );
					
					if (PC->PlayerState->UniqueId.IsValid())
					{				
						Events->TriggerEvent(*PC->PlayerState->UniqueId, TEXT("PlayerSessionStart"), Params);
					}
				}
			}
		}
	}
}

void AShooterGameMode::TriggerRoundEndForLocalPlayers()
{
	// Send start match event, this will set the CurrentMap stat.
	const auto Events = Online::GetEventsInterface();
	const auto Identity = Online::GetIdentityInterface();

	FOnlineEventParms Params;   
	FString MapName = *FPackageName::GetShortName(GetWorld()->PersistentLevel->GetOutermost()->GetName());

	if(Events.IsValid() && Identity.IsValid())
	{	
		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AShooterPlayerController* PC = Cast<AShooterPlayerController>(*It);
			ULocalPlayer* LocalPlayer = PC ? Cast<ULocalPlayer>(PC->Player) : nullptr;
			if (LocalPlayer)
			{					
				int32 UserIndex = LocalPlayer->ControllerId;
				if (UserIndex != -1)
				{	
					// round end
					{					
						FOnlineEventParms Params;
						Params.Add( TEXT( "GameplayModeId" ), FVariantData( (int32)1 ) );
						Params.Add( TEXT( "DifficultyLevelId" ), FVariantData( (int32)0 ) );
						Params.Add( TEXT( "ExitStatusId" ), FVariantData( (int32)0 ) );
						
						if (PC->PlayerState->UniqueId.IsValid())
						{				
							Events->TriggerEvent(*PC->PlayerState->UniqueId, TEXT("PlayerSessionEnd"), Params);
						}
					}					
				}
			}
		}
	}
}

void AShooterGameMode::RequestFinishAndExitToMainMenu()
{
	FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
	FString LocalReturnReason(TEXT(""));
	
	FinishMatch();

	UShooterGameKing::Get().RemoveSplitScreenPlayers(GetWorld());

	APlayerController * LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* Controller = *Iterator;
		if (Controller && !Controller->IsLocalController())
		{
			Controller->ClientReturnToMainMenu(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	if (LocalPrimaryController != NULL)
	{
		LocalPrimaryController->ClientReturnToMainMenu(LocalReturnReason);
	}
}

void AShooterGameMode::DetermineMatchWinner()
{
	// nothing to do here
}

bool AShooterGameMode::IsWinner(class AShooterPlayerState* PlayerState) const
{
	return false;
}

void AShooterGameMode::PreLogin(const FString& Options, const FString& Address, const TSharedPtr<FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	const FString EndGameError = TEXT("Match is over!");

	ErrorMessage = bMatchIsOver ? *EndGameError : GameSession->ApproveLogin(Options);
}


void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	AShooterPlayerController* NewPC = Cast<AShooterPlayerController>(NewPlayer);
	if (NewPC && NewPC->GetPawn() == NULL)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	}

	// start warmup if needed
	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime == 0)
	{
		const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
		if (bWantsMatchWarmup && WarmupTime > 0)
		{
			MyGameState->RemainingTime = WarmupTime;
		}
		else
		{
			MyGameState->RemainingTime = 0.0f;
		}
	}

	// notify new player if match is already in progress
	if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
	}
}

void AShooterGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	AShooterPlayerState* KillerPlayerState = Killer ? Cast<AShooterPlayerState>(Killer->PlayerState) : NULL;
	AShooterPlayerState* VictimPlayerState = KilledPlayer ? Cast<AShooterPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
}

float AShooterGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AShooterCharacter* DamagedPawn = Cast<AShooterCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AShooterPlayerState* DamagedPlayerState = Cast<AShooterPlayerState>(DamagedPawn->PlayerState);
		AShooterPlayerState* InstigatorPlayerState = Cast<AShooterPlayerState>(EventInstigator->PlayerState);

		// disable friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.0f;
		}

		// scale self instigated damage
		if (InstigatorPlayerState == DamagedPlayerState)
		{
			ActualDamage *= DamageSelfScale;
		}
	}

	return ActualDamage;
}

bool AShooterGameMode::CanDealDamage(class AShooterPlayerState* DamageInstigator, class AShooterPlayerState* DamagedPlayer) const
{
	return true;
}

bool AShooterGameMode::AllowCheats(APlayerController* P)
{
	return true;
}

bool AShooterGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

UClass* AShooterGameMode::GetDefaultPawnClassForController(AController* InController)
{
	if (Cast<AShooterAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController(InController);
}

AActor* AShooterGameMode::ChoosePlayerStart(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestSpawn = PlayerStarts[i];
		if (IsSpawnpointAllowed(TestSpawn, Player))
		{
			if (IsSpawnpointPreferred(TestSpawn, Player))
			{
				PreferredSpawns.Add(TestSpawn);
			}
			else
			{
				FallbackSpawns.Add(TestSpawn);
			}
		}
	}

	APlayerStart* BestStart = NULL;
	if (PreferredSpawns.Num() > 0)
	{
		BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
	}
	else if (FallbackSpawns.Num() > 0)
	{
		BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart(Player);
}

bool AShooterGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	AShooterTeamStart* ShooterSpawnPoint = Cast<AShooterTeamStart>(SpawnPoint);
	if (ShooterSpawnPoint)
	{
		AShooterAIController* AIController = Cast<AShooterAIController>(Player);
		if (ShooterSpawnPoint->bNotForBots && AIController)
		{
			return false;
		}

		if (ShooterSpawnPoint->bNotForPlayers && AIController == NULL)
		{
			return false;
		}
	}

	return true;
}

bool AShooterGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Player ? Cast<ACharacter>(Player->GetPawn()) : NULL;
	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->CapsuleComponent->GetScaledCapsuleHalfHeight() + OtherPawn->CapsuleComponent->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->CapsuleComponent->GetScaledCapsuleRadius() + OtherPawn->CapsuleComponent->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}
	}

	return true;
}

class AShooterBot* AShooterGameMode::SpawnBot(FVector SpawnLocation, FRotator SpawnRotation)
{
	if (BotPawnClass)
	{
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.bNoCollisionFail = true;
		AShooterBot* Bot = GetWorld()->SpawnActor<AShooterBot>(BotPawnClass, SpawnLocation, SpawnRotation, SpawnInfo);
		if (Bot)
		{
			Bot->SpawnDefaultController();
			return Bot;
		}
	}

	return NULL;
}

void AShooterGameMode::ConformToKingState()
{	
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}
	EShooterGameState CurrentKingState = UShooterGameKing::Get().GetCurrentState();
	if (CurrentKingState != CurrentState)
	{
		RequestFinishAndExitToMainMenu();
		CurrentState = CurrentKingState;
	}	
}

void AShooterGameMode::Tick( float DeltaSeconds )
{
	ConformToKingState();
}

void AShooterGameMode::SpawnBotsForGame()
{
	// getting max number of players
	int32 MaxPlayers = -1;
	if (GameSession)
	{
		MaxPlayers = GameSession->MaxPlayers;
	}	

	// checking number of human players
	int32 NumPlayers = 0;
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(*It);
		if (PC)
		{
			++NumPlayers;
		}
	}

	// adding bots
	BotControllers.Empty();
	int32 NumBots = 0;
	while (NumPlayers < MaxPlayers && NumBots < MaxBots)
	{
		AShooterBot* Bot = SpawnBot(FVector(ForceInitToZero), FRotator(ForceInitToZero));
		if (Bot)
		{
			InitBot(Bot, NumBots + 1);
			++NumBots;
		}
	}
}

void AShooterGameMode::InitBot(AShooterBot* Bot, int BotNumber)
{
	AShooterAIController* AIPC = Bot ? Cast<AShooterAIController>(Bot->GetController()) : NULL;
	if (AIPC)
	{
		if (AIPC->PlayerState)
		{
			FString BotName = FString::Printf(TEXT("Bot %d"), BotNumber);
			AIPC->PlayerState->PlayerName = BotName;
		}
		AActor* BestStart = ChoosePlayerStart(AIPC);

		UE_LOG(LogShooter, Log, TEXT("InitBot: %s spawned on %s"), *Bot->GetName(), *BestStart->GetName());
		Bot->TeleportTo(BestStart->GetActorLocation(), BestStart->GetActorRotation(), false, true);
	}
}
