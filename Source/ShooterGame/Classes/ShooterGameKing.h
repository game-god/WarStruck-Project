// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterGame.h"
#include "OnlineIdentityInterface.h"
#include "ShooterGameKing.generated.h"

/**
 * SHOOTERGAMEKING IS AN EXPERIMENTAL GAMESTATE MANAGEMENT SYSTEM.  THE CURRENT 'PULL' MODEL WITH CONFORMING STATE
 * IS BEING REWRITTEN TO WORK AS A PUSH MODEL, AND IS BEING INTEGRATED INTO THE ENGINE PROPER.  IT IS NOT RECOMMENDED
 * OR REQUIRED TO BASE OTHER GAMES ON THE FOLLOWING SYSTEM UNTIL THE REFACTORING IS COMPLETE.

 * ShooterGameKing represents the authoritative main state of the game.
 * It  accepts all game-state relevant
 * OS events and is responsible for making sure they are handled properly without being missed.
 * It acts as the authority on the desired state of the game.
 * Other systems like menus and OS events can request state changes, which all systems must react to.
 * We are testing out a polling based reaction mechanism for top-level game logic classes.
 * The intention is that it can be abstracted and built into the engine for every game to use as a way to manage
 * game state in a TRC compliant manner without nasty edge cases about missing OS events.
 */

/**
 * Main states of the game for the actual game-logic classes to compare themselves against.  e.g. (ShooterMainMenu, ShooterGameMode, etc)
 * Other states like Loading, or online lobby states can be added as necessary to properly react to OSS events and player input.
 */
enum class EShooterGameState : short
{	
	EWelcomeScreen, 	//initial screen.  Used for platforms where we may not have a signed in user yet.
	EMessageScreen, 	//message screen.  Used to display a message - EG unable to connect to game.
	EMainMenu,		//Main frontend state of the game.  No gameplay, just user/session management and UI.	
	EPlaying,		//Game should be playable, or loading into a playable state.
	EShutdown,		//Game is shutting down.
	EUnknown,		//unknown state. mostly for some initializing game-logic objects.
};

UCLASS(config=Game, notplaceable)
class UShooterGameKing : public UObject, public FTickerObjectBase
{
public:

	GENERATED_UCLASS_BODY()

	// Create Singleton instance.  Initialize in a specific location, not on first get so we error if we might have missed an OSS event.
	static void Initialize();
	
	// Grab the Singleton instance
	static UShooterGameKing& Get();

	// Gets the current master state of the game.
	EShooterGameState GetCurrentState() const;

	// Sets the current master state of the game.
	// would probably be replaced with more specific functions to move to specific states
	// in a more complicated game.
	virtual void SetCurrentState(EShooterGameState NewState);

public:
	//FTicker Funcs

	//do anything we need to frame to frame to make up for various legacy code issues.
	virtual bool Tick(float DeltaSeconds) OVERRIDE;	

public:
	//Game level utility functions

	// Removes all players in the world except for player 0.
	void RemoveSplitScreenPlayers(UWorld* WorldObj);

	// The initial screen is different on consoles and on PC.  Returns the appropriate state for the current platform.
	EShooterGameState GetInitialFrontendState() const;

private:	

	UShooterGameKing();
	void InitInternal();

	void HandleUserLoginChanged(int32 GameUserIndex);
	
	// Callback to pause the game when the OS has constrained our app.
	void HandleWillDeactivate();

	// Callback to handle safe frame size changes.
	void HandleSafeFrameChanged();

	// current desired gamestate.
	EShooterGameState CurrentState;

	// OSS delegates the King needs to handle
	FOnLoginChangedDelegate OnLoginChangedDelegate;	

	// singleton instance
	static UShooterGameKing* ShooterKingInstance;
};

FORCEINLINE UShooterGameKing& UShooterGameKing::Get()
{
	check(ShooterKingInstance != nullptr);
	return *ShooterKingInstance;
}

FORCEINLINE EShooterGameState UShooterGameKing::GetCurrentState() const
{
	return CurrentState;
}
