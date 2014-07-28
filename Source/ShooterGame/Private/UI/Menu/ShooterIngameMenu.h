// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "Slate.h"
#include "Widgets/ShooterMenuItem.h"
#include "Widgets/SShooterMenuWidget.h"
#include "ShooterOptions.h"

class FShooterIngameMenu : public TSharedFromThis<FShooterIngameMenu>
{
public:
	/** sets owning player controller */
	void Construct(AShooterPlayerController* _PCOwner);

	/** toggles in game menu */
	void ToggleGameMenu();

	/** is game menu currently active? */
	bool GetIsGameMenuUp() const;	

	/* Call UpdateMenuOwner on all menus widgets */
	void UpdateMenuOwner();

protected:

	/** Owning player controller */
	AShooterPlayerController* PCOwner;

	/** Owning User Index */
	int32 OwnerUserIndex;

	/** game menu container widget - used for removing */
	TSharedPtr<class SWeakWidget> GameMenuContainer;

	/** root menu item pointer */
	TSharedPtr<FShooterMenuItem> RootMenuItem;

	/** main menu item pointer */
	TSharedPtr<FShooterMenuItem> MainMenuItem;

	/** HUD menu widget */
	TSharedPtr<class SShooterMenuWidget> GameMenuWidget;	

	/** if game menu is currently opened*/
	bool bIsGameMenuUp;

	/** holds cheats menu item to toggle it's visibility */
	TSharedPtr<class FShooterMenuItem> CheatsMenu;

	/** Shooter options */
	TSharedPtr<class FShooterOptions> ShooterOptions;

	/** called when going back to previous menu */
	void OnMenuGoBack(MenuPtr Menu);
	
	/** goes back in menu structure */
	void CloseSubMenu();

	/** removes widget from viewport */
	void DetachGameMenu();
	
	/** Delegate called when user cancels confirmation dialog to exit to main menu */
	void OnCancelExitToMain();

	/** Delegate called when user confirms confirmation dialog to exit to main menu */
	void OnConfirmExitToMain();		

	/** Plays sound and calls Quit */
	void OnUIQuit();

	/** Quits the game */
	void Quit();
};