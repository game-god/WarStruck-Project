// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "Slate.h"

class FShooterWelcomeMenu : public TSharedFromThis<FShooterWelcomeMenu>
{
public:
	/** build menu */
	void Construct(class ULocalPlayer* InLocalPlayer, class AShooterGame_Menu* Game);

	/** Add the menu to the gameviewport so it becomes visible */
	void AddToGameViewport();

	/** Remove from the gameviewport. */
	void RemoveFromGameViewport();

	/**
	 * The delegate function for external login UI closure when a user has signed in.
	 *
	 * @param UniqueId The unique Id of the user who just signed in.
	 * @param ControllerIndex The controller index of the player who just signed in.
	 */
	void HandleLoginUIClosed(TSharedPtr<FUniqueNetId> UniqueId, const int ControllerIndex);

	/**
	 * Called when a user needs to advance from the welcome screen to the main menu.
	 *
	 * @param ControllerIndex The controller index of the player that's advancing.
	 */
	void SetControllerAndAdvanceToMainMenu(const int ControllerIndex);

private:
	/** Owning menu */
	AShooterGame_Menu* GameModeOwner;

	/** Owning local player */
	ULocalPlayer* LocalPlayer;

	/** Cache the user id that tried to advance, so we can use it again after the confirmation dialog */
	int PendingControllerIndex;

	/** "Presss A/X to play" widget */
	TSharedPtr<class SWidget> MenuWidget;

	/** Confirm continuing without a profile widget */
	TSharedPtr<class SShooterConfirmationDialog> ConfirmationWidget;

	/** Handler for continue-without-saving confirmation. */
	FReply OnContinueWithoutSavingConfirm();

	/** Handler for backing out of continue-without-saving. */
	FReply OnContinueWithoutSavingBack();
};
