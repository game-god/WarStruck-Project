// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#include "Online.h"
#include "ShooterLeaderboards.h"
#include "ShooterGameSession.generated.h"

struct FShooterGameSessionParams
{
	/** Name of session settings are stored with */
	FName SessionName;
	/** LAN Match */
	bool bIsLAN;
	/** Presence enabled session */
	bool bIsPresence;
	/** ControllerId of player initiating lobby */
	int32 ControllerId;
	/** Current search result choice to join */
	int32 BestSessionIdx;

	FShooterGameSessionParams()
		: SessionName(NAME_None)
		, bIsLAN(false)
		, bIsPresence(false)
		, ControllerId(0)
		, BestSessionIdx(0)
	{
	}
};

UCLASS(config=Game)
class SHOOTERGAME_API AShooterGameSession : public AGameSession
{
	GENERATED_UCLASS_BODY()

protected:

	/** Delegate for creating a new session */
	FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
	/** Delegate after starting a session */
	FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
	/** Delegate for destroying a session */
	FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
	/** Delegate for searching for sessions */
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	/** Delegate after joining a session */
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

	/** Transient properties of a session during game creation/matchmaking */
	FShooterGameSessionParams CurrentSessionParams;
	/** Current host settings */
	TSharedPtr<class FShooterOnlineSessionSettings> HostSettings;
	/** Current search settings */
	TSharedPtr<class FShooterOnlineSearchSettings> SearchSettings;

	/**
	 * Delegate fired when a session create request has completed
	 *
	 * @param SessionName the name of the session this callback is for
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 */
	virtual void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * Delegate fired when a session start request has completed
	 *
	 * @param SessionName the name of the session this callback is for
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 */
	void OnStartOnlineGameComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * Delegate fired when a session search query has completed
	 *
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 */
	void OnFindSessionsComplete(bool bWasSuccessful);

	/**
	 * Delegate fired when a session join request has completed
	 *
	 * @param SessionName the name of the session this callback is for
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 */
	void OnJoinSessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * Delegate fired when a destroying an online session has completed
	 *
	 * @param SessionName the name of the session this callback is for
	 * @param bWasSuccessful true if the async action completed without error, false if there was an error
	 */
	virtual void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	/**
	 * Safe delete mechanism to make sure we aren't deleting a session too soon after its creation
	 */
	void DelayedSessionDelete();

	/**
	 * Reset the variables the are keeping track of session join attempts
	 */
	void ResetBestSessionVars();

	/**
	 * Choose the best session from a list of search results based on game criteria
	 */
	void ChooseBestSession();

	/**
	 * Entry point for matchmaking after search results are returned
	 */
	void StartMatchmaking();

	/**
	 * Return point after each attempt to join a search result
	 */
	void ContinueMatchmaking();

	/**
	 * Delegate triggered when no more search results are available
	 */
	void OnNoMatchesAvailable();

	/* 
	 * Event triggered when a presence session is created
	 *
	 * @param SessionName name of session that was created
	 * @param bWasSuccessful was the create successful
	 */
	DECLARE_EVENT_TwoParams(AShooterGameSession, FOnCreatePresenceSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	FOnCreatePresenceSessionComplete CreatePresenceSessionCompleteEvent;

	/* 
	 * Event triggered when a session is joined
	 *
	 * @param SessionName name of session that was joined
	 * @param bWasSuccessful was the create successful
	 */
	//DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnJoinSessionComplete, FName /*SessionName*/, bool /*bWasSuccessful*/);
	DECLARE_EVENT_OneParam(AShooterGameSession, FOnJoinSessionComplete, bool /*bWasSuccessful*/);
	FOnJoinSessionComplete JoinSessionCompleteEvent;

	/*
	 * Event triggered after session search completes
	 */
	//DECLARE_EVENT(AShooterGameSession, FOnFindSessionsComplete);
	DECLARE_EVENT_OneParam(AShooterGameSession, FOnFindSessionsComplete, bool /*bWasSuccessful*/);
	FOnFindSessionsComplete FindSessionsCompleteEvent;

public:

	/** Default number of players allowed in a game */
	static const int32 DEFAULT_NUM_PLAYERS = 16;

	/**
	 * Host a new online session
	 *
	 * @param ControllerId controller that initiated the request
	 * @param SessionName name of session 
	 * @param bIsLAN is this going to hosted over LAN
	 * @param bIsPresence is the session to create a presence session
	 * @param MaxNumPlayers Maximum number of players to allow in the session
	 *
	 * @return bool true if successful, false otherwise
	 */
	bool HostSession(int32 ControllerId, FName SessionName, const FString & GameType, bool bIsLAN, bool bIsPresence, int32 MaxNumPlayers);

	/**
	 * Find an online session
	 *
	 * @param ControllerId controller that initiated the request
	 * @param SessionName name of session this search will generate
	 * @param bIsLAN are we searching LAN matches
	 * @param bIsPresence are we searching presence sessions
	 */
	void FindSessions(int32 ControllerId, FName SessionName, bool bIsLAN, bool bIsPresence);

	/**
	 * Joins one of the session in search results
	 *
	 * @param ControllerId controller that initiated the request
	 * @param SessionName name of session 
	 * @param SessionIndexInSearchResults Index of the session in search results
	 *
	 * @return bool true if successful, false otherwise
	 */
	bool JoinSession(int32 ControllerId, FName SessionName, int32 SessionIndexInSearchResults);

	/** @return true if any online async work is in progress, false otherwise */
	bool IsBusy() const;

	/**
	 * Get the search results found and the current search result being probed
	 *
	 * @param SearchResultIdx idx of current search result accessed
	 * @param NumSearchResults number of total search results found in FindGame()
	 *
	 * @return State of search result query
	 */
	EOnlineAsyncTaskState::Type GetSearchResultStatus(int32& SearchResultIdx, int32& NumSearchResults);

	/**
	 * Get the search results.
	 *
	 * @return Search results
	 */
	const TArray<FOnlineSessionSearchResult> & GetSearchResults() const;

	/** @return the delegate fired when creating a presence session */
	FOnCreatePresenceSessionComplete& OnCreatePresenceSessionComplete() { return CreatePresenceSessionCompleteEvent; }

	/** @return the delegate fired when joining a session */
	FOnJoinSessionComplete& OnJoinSessionComplete() { return JoinSessionCompleteEvent; }

	/** @return the delegate fired when search of session completes */
	FOnFindSessionsComplete& OnFindSessionsComplete() { return FindSessionsCompleteEvent; }

	/** 
	 * Creates a game session 
	 *
	 * @param ControllerId id of the controller that owns this session
	 */
	virtual void CreateGameSession(int32 ControllerId);

	/** Handle starting the match */
	virtual void HandleMatchHasStarted() OVERRIDE;

	/** Handles when the match has ended */
	virtual void HandleMatchHasEnded() OVERRIDE;

	/** 
	 * Destroys a game session 
	 *
	 * @param ControllerId id of the controller that owns this session
	 */
	virtual void DestroyGameSession(int32 ControllerId);

	/**
	 * Travel to a session URL (as client) for a given session
	 *
	 * @param ControllerId controller initiating the session travel
	 * @param SessionName name of session to travel to
	 *
	 * @return true if successful, false otherwise
	 */
	bool TravelToSession(int32 ControllerId, FName SessionName);
};

