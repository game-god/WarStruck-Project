// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "OnlineIdentityInterface.h"
#include "ShooterGameMode.generated.h"

enum class EShooterGameState : short;

UCLASS(config=Game)
class AShooterGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()

	/** The bot pawn class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=GameMode)
	TSubclassOf<class APawn> BotPawnClass;

	UFUNCTION(exec)
	void SetAllowBots(bool bInAllowBots, int32 InMaxBots = 8);

	/** Initialize the game. This is called before actors' PreInitializeComponents. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) OVERRIDE;

	/** Accept or reject a player attempting to join the server.  Fails login if you set the ErrorMessage to a non-empty string. */
	virtual void PreLogin(const FString& Options, const FString& Address, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage) OVERRIDE;

	/** starts match warmup */
	virtual void PostLogin(APlayerController* NewPlayer) OVERRIDE;

	/** select best spawn point for player */
	virtual AActor* ChoosePlayerStart(AController* Player) OVERRIDE;

	/** always pick new random spawn */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) OVERRIDE;

	/** returns default pawn class for given controller */
	virtual UClass* GetDefaultPawnClassForController(AController* InController) OVERRIDE;

	/** prevents friendly fire */
	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	/** notify about kills */
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	/** can players damage each other? */
	virtual bool CanDealDamage(class AShooterPlayerState* DamageInstigator, class AShooterPlayerState* DamagedPlayer) const;

	/** always create cheat manager */
	virtual bool AllowCheats(APlayerController* P) OVERRIDE;

	/** update remaining time */
	virtual void DefaultTimer() OVERRIDE;

	/** starts new match */
	virtual void HandleMatchHasStarted() OVERRIDE;

	/** spawns default bot */
	class AShooterBot* SpawnBot(FVector SpawnLocation, FRotator SpawnRotation);	

	virtual void ConformToKingState();

	virtual void Tick( float DeltaSeconds );

protected:

	/** delay between first player login and starting match */
	UPROPERTY(config)
	int32 WarmupTime;

	/** match duration */
	UPROPERTY(config)
	int32 RoundTime;

	UPROPERTY(config)
	int32 TimeBetweenMatches;

	/** score for kill */
	UPROPERTY(config)
	int32 KillScore;

	/** score for death */
	UPROPERTY(config)
	int32 DeathScore;

	/** scale for self instigated damage */
	UPROPERTY(config)
	float DamageSelfScale;

	UPROPERTY(config)
	int32 MaxBots;

	UPROPERTY()
	TArray<AShooterAIController*> BotControllers;
	
	EShooterGameState CurrentState;

	bool bAllowBots;		

	/** Triggers round start event for local players. Needs revising when shootergame goes multiplayer */
	void TriggerRoundStartForLocalPlayers();

	/** Triggers round end event for local players. Needs revising when shootergame goes multiplayer */
	void TriggerRoundEndForLocalPlayers();

	/** spawning all bots for this game */
	void SpawnBotsForGame();

	/** initialization for bot after spawning */
	virtual void InitBot(AShooterBot* Bot, int BotNumber);

	/** check who won */
	virtual void DetermineMatchWinner();

	/** check if PlayerState is a winner */
	virtual bool IsWinner(class AShooterPlayerState* PlayerState) const;

	/** check if player can use spawnpoint */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	/** check if player should use spawnpoint */
	virtual bool IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const OVERRIDE;	

public:

	/** finish current match and lock players */
	UFUNCTION(exec)
	void FinishMatch();

	/*Finishes the match and bumps everyone to main menu.*/
	void RequestFinishAndExitToMainMenu();

	/** get the name of the bots count option used in server travel URL */
	static FString GetBotsCountOptionName();

	UPROPERTY()
	TArray<class AShooterPickup*> LevelPickups;

};
