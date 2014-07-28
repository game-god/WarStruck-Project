// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "Slate.h"
#include "Widgets/ShooterMenuItem.h"
#include "Widgets/SShooterMenuWidget.h"
#include "Widgets/SShooterServerList.h"
#include "Widgets/SShooterLeaderboard.h"
#include "Widgets/SShooterSplitScreenLobbyWidget.h"
#include "ShooterOptions.h"
#include "ShooterGameKing.h"


class FShooterMainMenu : public TSharedFromThis<FShooterMainMenu>, public FTickableGameObject
{
public:	

	virtual ~FShooterMainMenu();

	/** build menu */
	void Construct(APlayerController* _PCOwner, AShooterGame_Menu* _SGOwner);

	/** Add the menu to the gameviewport so it becomes visible */
	void AddMenuToGameViewport();

	/** Remove from the gameviewport. */
	void RemoveMenuFromGameViewport();	

	/** TickableObject Functions */
	virtual void Tick(float DeltaTime) OVERRIDE;	
	virtual bool IsTickable() const OVERRIDE;	
	virtual TStatId GetStatId() const OVERRIDE;
	virtual bool IsTickableWhenPaused() const OVERRIDE;	


protected:

	enum class EMap
	{
		ESancturary,
		EHighRise,
		EMax,
	};	
	
	/** Owning player controller */
	APlayerController* PCOwner;

	/** Owning User Index */
	int32 OwnerUserIndex;

	/** Owning menu */
	AShooterGame_Menu* SGOwner;

	/** shooter options */
	TSharedPtr<class FShooterOptions> ShooterOptions;

	/** menu widget */
	TSharedPtr<class SShooterMenuWidget> MenuWidget;

	/* used for removing the MenuWidget */
	TSharedPtr<class SWeakWidget> MenuWidgetContainer;

	/** SplitScreen Lobby Widget */
	TSharedPtr<class SShooterSplitScreenLobby> SplitScreenLobbyWidget;

	/* used for removing the SplitScreenLobby */
	TSharedPtr<class SWeakWidget> SplitScreenLobbyWidgetContainer;

	/** server list widget */
	TSharedPtr<class SShooterServerList> ServerListWidget;

	/** leaderboard widget */
	TSharedPtr<class SShooterLeaderboard> LeaderboardWidget;

	/** custom menu */
	TSharedPtr<class FShooterMenuItem> JoinServerItem;

	/** yet another custom menu */
	TSharedPtr<class FShooterMenuItem> LeaderboardItem;

	/** LAN Options */
	TSharedPtr<class FShooterMenuItem> HostLANItem;
	TSharedPtr<class FShooterMenuItem> JoinLANItem;	

	/** Map selection widget */
	TSharedPtr<FShooterMenuItem> MapOption;

	/** Track if we are showing a map download pct or not. */
	bool bShowingDownloadPct;

	EMap GetSelectedMap() const;

	/** goes back in menu structure */
	void CloseSubMenu();

	/** called when going back to previous menu */
	void OnMenuGoBack(MenuPtr Menu);

	/** called when menu hide animation is finished */
	void OnMenuHidden();

	/** called when user chooses split screen.  Goes to the split screen setup screen.  Hides menu widget*/
	void OnSplitScreenSelected();

	/** called when users back out of split screen lobby screen.  Shows main menu again. */
	void SplitScreenBackedOut();

	FReply OnSplitScreenBackedOut();

	FReply OnSplitScreenPlay();

	/** bot count option changed callback */
	void BotCountOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex);			

	/** lan match option changed callback */
	void LanMatchChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex);

	/** Plays StartGameSound sound and calls HostFreeForAll after sound is played */
	void OnUIHostFreeForAll();

	/** Plays StartGameSound sound and calls HostTeamDeathMatch after sound is played */
	void OnUIHostTeamDeathMatch();

	/** Hosts free for all game */
	void HostFreeForAll();

	/** Hosts team deathmatch game */
	void HostTeamDeathMatch();

	/** Creates any ready SplitScreen Players */
	void CreateSplitScreenPlayers();	

	/** Join server */
	void OnJoinServer();

	/** Show leaderboard */
	void OnShowLeaderboard();

	/** Plays sound and calls Quit */
	void OnUIQuit();

	/** Quits the game */
	void Quit();

	/** Lock the controls and hide the main menu */
	void LockAndHideMenu();

	/** Display the loading screen. */
	void DisplayLoadingScreen();

	/** Updates the OwnerIndex from the player controller */
	void UpdateMenuOwner();

	/** Checks the ChunkInstaller to see if the selected map is ready for play */
	bool IsMapReady() const;

	/** Get the persistence user associated with PCOwner*/
	UShooterPersistentUser* GetPersistentUser() const;

	/** number of bots in game */
	int32 BotsCountOpt;

	/** lan game? */
	bool bIsLanMatch;	
};
