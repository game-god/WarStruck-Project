// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame_TeamDeathMatch.generated.h"

UCLASS()
class AShooterGame_TeamDeathMatch : public AShooterGameMode
{
	GENERATED_UCLASS_BODY()

	/** initialize player */
	virtual void InitNewPlayer(AController* NewPlayer, const TSharedPtr<FUniqueNetId>& UniqueId, const FString& Options) OVERRIDE;

	/** initialize replicated game data */
	virtual void InitGameState() OVERRIDE;

	/** can players damage each other? */
	virtual bool CanDealDamage(class AShooterPlayerState* DamageInstigator, class AShooterPlayerState* DamagedPlayer) const OVERRIDE;

protected:

	/** number of teams */
	int32 NumTeams;

	/** best team */
	int32 WinnerTeam;

	/** pick team with least players in or random when it's equal */
	int32 ChooseTeam(class AShooterPlayerState* ForPlayerState) const;

	/** check who won */
	virtual void DetermineMatchWinner() OVERRIDE;

	/** check if PlayerState is a winner */
	virtual bool IsWinner(class AShooterPlayerState* PlayerState) const OVERRIDE;

	/** check team constraints */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	/** initialization for bot after spawning */
	virtual void InitBot(AShooterBot* Bot, int BotNumber) OVERRIDE;
};
