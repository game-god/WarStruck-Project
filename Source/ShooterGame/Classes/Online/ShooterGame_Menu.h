// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame_Menu.generated.h"

enum class EShooterGameState : short;

UCLASS()
class AShooterGame_Menu : public AGameMode
{
	GENERATED_UCLASS_BODY()

public:

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Controller id that initiated session join */
	int32 JoiningControllerId;

	// Begin AGameMode interface
	/** skip it, menu doesn't require player start or pawn */
	virtual void RestartPlayer(class AController* NewPlayer) OVERRIDE;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const OVERRIDE;

	/** Menu game is ending.  Update King state accordingly */
	virtual void HandleLeavingMap() OVERRIDE;

	// End AGameMode interface

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Callback which is intended to be called upon session creation */
	void OnCreateGameSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionsComplete(bool bWasSuccessful);

	/** Callback which is intended to be called upon joining session */
	void OnJoinSessionComplete(bool bWasSuccessful);

	/** Create game session */
	void StartGameSession(APlayerController* PCOwner, const FString& GameType);

	/** Starts the game */
	bool HostGame(APlayerController* PCOwner, const FString& GameType, const FString& InTravelURL);

	/** Initiates the session searching */
	bool FindSessions(APlayerController* PCOwner, bool bLANMatch);

	/** Joins one of sessions previously found */
	bool JoinSession(APlayerController* PCOwner, int32 SessionIndexInSearchResults);

	// Begin AActor interface
	virtual void BeginPlay() OVERRIDE;
	// End AActor interface	

	/* Add handlers for failing network/travel failures */
	void AddFailureHandlers();

protected:

	/** Perform some final tasks before hosting/joining a session. Remove menus, set king state etc */
	void BeginSession();
	
	/**
	* Creates the main menu, clears the welcome screen, and sets the KingState to MainMenu
	*/
	void ToMainMenu();

	/** Creates the welcome screen, clears the main menu, and sets the KingState to WelcomeScreen */
	void ToWelcome();

	/** 
	 * Creates the message menu, clears other menus and sets the KingState to Message.
	 *
	 * @param	Message				Main message body
	 * @param	OKButtonString		String to use for 'OK' button
	 * @param	CancelButtonString	String to use for 'Cancel' button
	 */
	void ShowMessageThenGoMain(const FString& Message, const FString& OKButtonString, const FString& CancelButtonString);

	/** Display a loading screen */
	void ShowLoadingScreen();

	/** Called when there is an error trying to join a local session */
	void JoinLocalSessionFailure(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

	/** Called when there is an error trying to travel to a local session */
	void TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ErrorString);

	/*
	 * Setup a state to inform the user of failing to connect/travel to a session 
	 *
	 * @param	InFailureReason		Reason for failure
	 */	
	void ShowJoinFailureMessageAndGoMain(const FString& InFailureReason);

	/** Clears the pointer to the main menu, which should destroy it since no one else should be holding a pointer to it. Removes from GameViewport */
	void ClearMainMenu();

	/** Clears the pointer to the welcome menu, which should destroy it since no one else should be holding a pointer to it. Removes from GameViewport */
	void ClearWelcomeMenu();

	/** Clears the pointer to the message menu, which should destroy it since no one else should be holding a pointer to it. Removes from GameViewport */
	void ClearMessageMenu();

	void ConformToKingState();

	virtual void Tick(float DeltaSeconds) OVERRIDE;	

	EShooterGameState CurrentState;

	/** Main menu UI */
	TSharedPtr<class FShooterMainMenu> ShooterMainMenuUI;

	/** Message menu (Shown in the even of errors - unable to connect etc) */
	TSharedPtr<class FShooterMessageMenu> MessageMenuUI;

	/** Welcome menu UI (for consoles) */
	TSharedPtr<class FShooterWelcomeMenu> ShooterWelcomeMenuUI;
};
