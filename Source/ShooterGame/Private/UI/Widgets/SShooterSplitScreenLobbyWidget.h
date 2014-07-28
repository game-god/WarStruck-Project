// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Slate.h"

class SShooterSplitScreenLobby : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS( SShooterSplitScreenLobby )
	{}

	SLATE_ARGUMENT(APlayerController*, LocalPlayer)
		
	SLATE_ARGUMENT(FOnClicked, OnPlayClicked)
	SLATE_ARGUMENT(FOnClicked, OnCancelClicked)

	SLATE_END_ARGS()	

	/** says that we can support keyboard focus */
	virtual bool SupportsKeyboardFocus() const OVERRIDE { return true; }

	void Construct(const FArguments& InArgs);

	void Clear();

	int32 GetNumSupportedSlots() const;
	int32 GetNumReadyPlayers() const;
	int32 GetUserIndexForSlot(int32 LobbySlot) const;

private:

	void OnLoginChanged(int32 LocalUserNum);

	virtual FReply OnControllerButtonPressed( const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent ) OVERRIDE;
	virtual FReply OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent) OVERRIDE;
	virtual void OnKeyboardFocusLost( const FKeyboardFocusEvent& InKeyboardFocusEvent ) OVERRIDE;

	void ReadyPlayer(TSharedPtr<FUniqueNetId> User, int UserIndex, int32 OpenSlot);
	void UnreadyPlayer(TSharedPtr<FUniqueNetId> User);

	void HandleLoginUIClosedAndReady(TSharedPtr<FUniqueNetId> UniqueId, const int UserIndex, const int SlotToJoin);

	void UpdateLobbyOwner();

	bool ConfirmSponsorsSatisfied() const;

	// Need both of these due to function pointer madness
	FSlateColor ErrorMessageSlateColor() const;
	FLinearColor ErrorMessageLinearColor() const;

	static const int MAX_POSSIBLE_SLOTS = 4;

	double TimeOfError;
	

	/** The player that owns the Lobby. */
	APlayerController* LocalPlayer;

	FOnClicked MasterUserBack;
	FOnClicked MasterUserPlay;
	
	TSharedPtr<FUniqueNetId> LobbyOwner;
	TSharedPtr<FUniqueNetId> Users[MAX_POSSIBLE_SLOTS];

	int32 LobbyOwnerUserIndex;
	int32 NumSupportedSlots;  //currently supported slots.  up to MAX_POSSIBLE_SLOTS
	int32 UserIndices[MAX_POSSIBLE_SLOTS];
	TSharedPtr<STextBlock> UserTextWidgets[MAX_POSSIBLE_SLOTS];
	TSharedPtr<SWidget> UserSlots[MAX_POSSIBLE_SLOTS];

	FText PressToPlayText;
	FText PressToStartMatchText;

	FOnLoginChangedDelegate OnLoginChangedDelegate;
};
