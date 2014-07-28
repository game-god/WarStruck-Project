// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameKing.h"

AShooterGameState::AShooterGameState(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	NumTeams = 0;
	RemainingTime = 0;
	bTimerPaused = false;

	// need to tick when paused to check king state.
	PrimaryActorTick.bCanEverTick = true;
	SetTickableWhenPaused(true);

	CurrentState = EShooterGameState::EPlaying;
}

void AShooterGameState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AShooterGameState, NumTeams );
	DOREPLIFETIME( AShooterGameState, RemainingTime );
	DOREPLIFETIME( AShooterGameState, bTimerPaused );
	DOREPLIFETIME( AShooterGameState, TeamScores );
}

void AShooterGameState::GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const
{
	OutRankedMap.Empty();

	//first, we need to go over all the PlayerStates, grab their score, and rank them
	TMultiMap<int32, AShooterPlayerState*> SortedMap;
	for(int32 i = 0; i < PlayerArray.Num(); ++i)
	{
		int32 Score = 0;
		AShooterPlayerState* CurPlayerState = Cast<AShooterPlayerState>(PlayerArray[i]);
		if (CurPlayerState && (CurPlayerState->GetTeamNum() == TeamIndex))
		{
			SortedMap.Add(FMath::TruncToInt(CurPlayerState->Score), CurPlayerState);
		}
	}

	//sort by the keys
	SortedMap.KeySort(TGreater<int32>());

	//now, add them back to the ranked map
	OutRankedMap.Empty();

	int32 Rank = 0;
	for(TMultiMap<int32, AShooterPlayerState*>::TIterator It(SortedMap); It; ++It)
	{
		OutRankedMap.Add(Rank++, It.Value());
	}
	
}

void AShooterGameState::ConformToKingState()
{
	// if AuthorityGameMode is non-null, then we are the server, and the game mode will handle
	// kicking the game back to the front end if necessary.  Otherwise, gamestate will need to handle that.
	if (AuthorityGameMode == nullptr)
	{	
		UShooterGameKing& ShooterGameKing = UShooterGameKing::Get();
		EShooterGameState CurrentKingState = ShooterGameKing.GetCurrentState();
		if (CurrentKingState != CurrentState)
		{
			ShooterGameKing.RemoveSplitScreenPlayers(GetWorld());			

			for(FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
			{
				// only remove local split screen controllers, so that the later loop still sends good messages to any remote controllers.
				APlayerController* Controller = *Iterator;
				if(Controller && Controller->IsLocalController() && Controller->IsPrimaryPlayer())
				{
					check(Controller->GetNetMode() == ENetMode::NM_Client);
					Controller->ClientReturnToMainMenu(TEXT("Return to Main Menu requested by game state"));
					break;
				}
			}
			CurrentState = CurrentKingState;
		}
	}
}

void AShooterGameState::Tick( float DeltaSeconds )
{
	ConformToKingState();
}