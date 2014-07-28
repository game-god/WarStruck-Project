// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Slate.h"
#include "ShooterGame.h"
#include "SShooterMenuWidget.h"

struct FServerEntry
{
	FString ServerName;
	FString CurrentPlayers;
	FString MaxPlayers;
	FString GameType;
	FString Ping;
	int32 SearchResultsIndex;
};

//class declare
class SShooterServerList : public SShooterMenuWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterServerList)
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
	void OnListItemDoubleClicked(TSharedPtr<FServerEntry> InItem);

	/** creates single item widget, called for every list item */
	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FServerEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** selection changed handler */
	void EntrySelectionChanged(TSharedPtr<FServerEntry> InItem, ESelectInfo::Type SelectInfo);

	/** 
	 * Get the current game
	 *
	 * @return The current game
	 */
	AShooterGame_Menu * GetGame() const;

	/** 
	 * Get the current game session
	 *
	 * @return The current game session
	 */
	AShooterGameSession* GetGameSession() const;

	/** Updates current search status */
	void UpdateSearchStatus();

	/** Starts searching for servers */
	void BeginServerSearch(bool bLANMatch);

	/** Called when server search is finished */
	void OnServerSearchFinished();

	/** fill/update server list, should be called before showing this control */
	void UpdateServerList();

	/** connect to chosen server */
	void ConnectToServer();

	/** selects item at current + MoveBy index */
	void MoveSelection(int32 MoveBy);

	/**
	 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
	 *
	 * @param  AllottedGeometry The space allotted for this widget
	 * @param  InCurrentTime  Current absolute real time
	 * @param  InDeltaTime  Real time passed since last tick
	 */
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime );

protected:

	/** Whether last searched for LAN (so spacebar works) */
	bool bLANMatchSearch;

	/** Whether we're searching for servers */
	bool bSearchingForServers;

	/** action bindings array */
	TArray< TSharedPtr<FServerEntry> > ServerList;

	/** action bindings list slate widget */
	TSharedPtr< SListView< TSharedPtr<FServerEntry> > > ServerListWidget; 

	/** currently selected list item */
	TSharedPtr<FServerEntry> SelectedItem;

	/** get current status text */
	FString GetBottomText() const;

	/** current status text */
	FString StatusText;

	/** size of standard column in pixels */
	int32 BoxWidth;

	/** pointer to our owner PC */
	TWeakObjectPtr<class APlayerController> PCOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;
};


