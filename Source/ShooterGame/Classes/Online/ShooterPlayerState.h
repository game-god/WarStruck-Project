// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterPlayerState.generated.h"

UCLASS()
class AShooterPlayerState : public APlayerState
{
	GENERATED_UCLASS_BODY()


	// Begin APlayerState interface
	/** clear scores */
	virtual void Reset() OVERRIDE;

	/**
	 * Set the team 
	 *
	 * @param	InController	The controller to initialize state with
	 */
	virtual void ClientInitialize(class AController* InController) OVERRIDE;

	// End APlayerState interface

	/**
	 * Set new team and update pawn. Also updates player character team colors.
	 *
	 * @param	NewTeamNumber	Team we want to be on.
	 */
	void SetTeamNum(int32 NewTeamNumber);

	/** player killed someone */
	void ScoreKill(AShooterPlayerState* Victim, int32 Points);

	/** player died */
	void ScoreDeath(AShooterPlayerState* KilledBy, int32 Points);

	/** get current team */
	int32 GetTeamNum() const;

	/** get number of kills */
	int32 GetKills() const;

	/** get number of deaths */
	int32 GetDeaths() const;

	/** get number of points */
	float GetScore() const;

	/** get number of bullets fired this match */
	int32 GetNumBulletsFired() const;

	/** get number of rockets fired this match */
	int32 GetNumRocketsFired() const;

	/** gets truncated player name to fit in death log and scoreboards */
	FString GetShortPlayerName() const;

	/** Sends kill (excluding self) to clients */
	UFUNCTION(Reliable, Client)
	void InformAboutKill(class AShooterPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AShooterPlayerState* KilledPlayerState);

	/** broadcast death to local clients */
	UFUNCTION(Reliable, NetMulticast)
	void BroadcastDeath(class AShooterPlayerState* KillerPlayerState, const UDamageType* KillerDamageType, class AShooterPlayerState* KilledPlayerState);

	/** replicate team colors. Updated the players mesh colors appropriately */
	UFUNCTION()
	void OnRep_TeamColor();

	//We don't need stats about amount of ammo fired to be server authenticated, so just increment these with local functions
	void AddBulletsFired(int32 NumBullets);
	void AddRocketsFired(int32 NumRockets);
protected:

	/** Set the mesh colors based on the current teamnum variable */
	void UpdateTeamColors();

	/** team number */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_TeamColor)
	int32 TeamNumber;

	/** number of kills */
	UPROPERTY(Transient, Replicated)
	int32 NumKills;

	/** number of deaths */
	UPROPERTY(Transient, Replicated)
	int32 NumDeaths;

	/** number of bullets fired this match */
	UPROPERTY()
	int32 NumBulletsFired;

	/** number of rockets fired this match */
	UPROPERTY()
	int32 NumRocketsFired;

	/** helper for scoring points */
	void ScorePoints(int32 Points);
};
