// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Slate.h"
#include "ShooterGame.h"
#include "ShooterLeaderboards.h"

/** leaderboard row display information */
struct FLeaderboardRow
{
	/** player rank*/
	FString Rank;

	/** player name */
	FString PlayerName;

	/** player total kills to display */
	FString Kills;

	/** player total deaths to display */
	FString Deaths;
};

//class declare
class SShooterLeaderboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterLeaderboard)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, PCOwner)
	SLATE_ARGUMENT(TSharedPtr<SWidget>, OwnerWidget)

	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	/** if we want to receive focus */
	virtual bool SupportsKeyboardFocus() const OVERRIDE { return true; }

	/** focus received handler - keep the ActionBindingsList focused */
	virtual FReply OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent) OVERRIDE;
	
	/** focus lost handler - keep the ActionBindingsList focused */
	virtual void OnKeyboardFocusLost( const FKeyboardFocusEvent& InKeyboardFocusEvent ) OVERRIDE;

	/** key down handler */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent) OVERRIDE;

	/** Called when a controller button is pressed */
	virtual FReply OnControllerButtonPressed( const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent ) OVERRIDE;

	/** SListView item double clicked */
	void OnListItemDoubleClicked(TSharedPtr<FLeaderboardRow> InItem);

	/** creates single item widget, called for every list item */
	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FLeaderboardRow> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** selection changed handler */
	void EntrySelectionChanged(TSharedPtr<FLeaderboardRow> InItem, ESelectInfo::Type SelectInfo);

	/** 
	 * Get the current game
	 *
	 * @return The current game
	 */
	AShooterGameMode * GetGame() const;

	/** 
	 * Get the current game session
	 *
	 * @return The current game session
	 */
	AShooterGameSession* GetGameSession() const;

	/** Starts reading leaderboards for the game */
	void ReadStats();

	/** Called on a particular leaderboard read */
	void OnStatsRead(bool bWasSuccessful);

	/** selects item at current + MoveBy index */
	void MoveSelection(int32 MoveBy);

protected:

	/** action bindings array */
	TArray< TSharedPtr<FLeaderboardRow> > StatRows;

	/** Leaderboard read object */
	FOnlineLeaderboardReadPtr ReadObject;

	/** Indicates that a stats read operation has been initiated */
	bool bReadingStats;

	/** Delegate called when a leaderboard has been successfully read */
	FOnLeaderboardReadCompleteDelegate LeaderboardReadCompleteDelegate;

	/** action bindings list slate widget */
	TSharedPtr< SListView< TSharedPtr<FLeaderboardRow> > > RowListWidget; 

	/** currently selected list item */
	TSharedPtr<FLeaderboardRow> SelectedItem;

	/** pointer to our owner PC */
	TWeakObjectPtr<class APlayerController> PCOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;
};


