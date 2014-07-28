// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"

AShooterPlayerState::AShooterPlayerState(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	TeamNumber = 0;
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
}

void AShooterPlayerState::Reset()
{
	Super::Reset();
	
	SetTeamNum(0);
	NumKills = 0;
	NumDeaths = 0;
	NumBulletsFired = 0;
	NumRocketsFired = 0;
}

void AShooterPlayerState::ClientInitialize(class AController* InController)
{
	Super::ClientInitialize(InController);

	UpdateTeamColors();
}

void AShooterPlayerState::SetTeamNum(int32 NewTeamNumber)
{
	TeamNumber = NewTeamNumber;

	UpdateTeamColors();
}

void AShooterPlayerState::OnRep_TeamColor()
{
	UpdateTeamColors();
}

void AShooterPlayerState::AddBulletsFired(int32 NumBullets)
{
	NumBulletsFired += NumBullets;
}

void AShooterPlayerState::AddRocketsFired(int32 NumRockets)
{
	NumRocketsFired += NumRockets;
}

void AShooterPlayerState::UpdateTeamColors()
{
	AController* OwnerController = Cast<AController>(GetOwner());
	if (OwnerController != NULL)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OwnerController->GetCharacter());
		if (ShooterCharacter != NULL)
		{
			ShooterCharacter->UpdateTeamColorsAllMIDs();
		}
	}
}

int32 AShooterPlayerState::GetTeamNum() const
{
	return TeamNumber;
}

int32 AShooterPlayerState::GetKills() const
{
	return NumKills;
}

int32 AShooterPlayerState::GetDeaths() const
{
	return NumDeaths;
}

float AShooterPlayerState::GetScore() const
{
	return Score;
}

int32 AShooterPlayerState::GetNumBulletsFired() const
{
	return NumBulletsFired;
}

int32 AShooterPlayerState::GetNumRocketsFired() const
{
	return NumRocketsFired;
}

void AShooterPlayerState::ScoreKill(AShooterPlayerState* Victim, int32 Points)
{
	NumKills++;
	ScorePoints(Points);
}

void AShooterPlayerState::ScoreDeath(AShooterPlayerState* KilledBy, int32 Points)
{
	NumDeaths++;
	ScorePoints(Points);
}

void AShooterPlayerState::ScorePoints(int32 Points)
{
	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GetWorld()->GameState);
	if (MyGameState && TeamNumber >= 0)
	{
		if (TeamNumber >= MyGameState->TeamScores.Num())
		{
			MyGameState->TeamScores.AddZeroed(TeamNumber - MyGameState->TeamScores.Num() + 1);
		}

		MyGameState->TeamScores[TeamNumber] += Points;
	}

	Score += Points;
}

void AShooterPlayerState::InformAboutKill_Implementation(class AShooterPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AShooterPlayerState* KilledPlayerState)
{
	//id can be null for bots
	if (KillerPlayerState->UniqueId.IsValid())
	{	
		//search for the actual killer before calling OnKill()	
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{		
			AShooterPlayerController* TestPC = Cast<AShooterPlayerController>(*It);
			if (TestPC && TestPC->IsLocalController())
			{
				// a local player might not have an ID if it was created with CreateDebugPlayer.
				ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(TestPC->Player);
				TSharedPtr<FUniqueNetId> LocalID = LocalPlayer->GetUniqueNetId();
				if (LocalID.IsValid() &&  *LocalPlayer->GetUniqueNetId() == *KillerPlayerState->UniqueId)
				{			
					TestPC->OnKill();
				}
			}
		}
	}
}

void AShooterPlayerState::BroadcastDeath_Implementation(class AShooterPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AShooterPlayerState* KilledPlayerState)
{	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		// all local players get death messages so they can update their huds.
		AShooterPlayerController* TestPC = Cast<AShooterPlayerController>(*It);
		if (TestPC && TestPC->IsLocalController())
		{
			TestPC->OnDeathMessage(KillerPlayerState, this, KillerDamageType);				
		}
	}	
}

void AShooterPlayerState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME( AShooterPlayerState, TeamNumber );
	DOREPLIFETIME( AShooterPlayerState, NumKills );
	DOREPLIFETIME( AShooterPlayerState, NumDeaths );
}

FString AShooterPlayerState::GetShortPlayerName() const
{
	return PlayerName.Left(MAX_PLAYER_NAME_LENGTH);
}