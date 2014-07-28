// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterIngameMenu.h"
#include "ShooterStyle.h"
#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterGameKing.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterIngameMenu::Construct(AShooterPlayerController* _PCOwner)
{
	PCOwner = _PCOwner;
	bIsGameMenuUp = false;

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}

	if (!GameMenuWidget.IsValid())
	{
		SAssignNew(GameMenuWidget, SShooterMenuWidget)
			.PCOwner(TWeakObjectPtr<APlayerController>(PCOwner))
			.Cursor(EMouseCursor::Default)
			.IsGameMenu(true);			


		// setup the exit to main menu submenu.  We wanted a confirmation to avoid a potential TRC violation.
		// fixes TTP: 322267
		TSharedPtr<FShooterMenuItem> MainMenuRoot = FShooterMenuItem::CreateRoot();
		MainMenuItem = MenuHelper::AddMenuItem(MainMenuRoot,LOCTEXT("Main Menu", "MAIN MENU"));
		MenuHelper::AddMenuItemSP(MainMenuItem,LOCTEXT("No", "NO"), this, &FShooterIngameMenu::OnCancelExitToMain);
		MenuHelper::AddMenuItemSP(MainMenuItem,LOCTEXT("Yes", "YES"), this, &FShooterIngameMenu::OnConfirmExitToMain);		

		ShooterOptions = MakeShareable(new FShooterOptions());
		ShooterOptions->Construct(PCOwner);
		ShooterOptions->TellInputAboutKeybindings();
		ShooterOptions->OnApplyChanges.BindSP(this, &FShooterIngameMenu::CloseSubMenu);

		/** root menu item pointer */
		MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterOptions->CheatsItem.ToSharedRef());
		MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterOptions->OptionsItem.ToSharedRef());

		if(FSlateApplication::Get().SupportsSystemHelp())
		{
			TSharedPtr<FShooterMenuItem> HelpSubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Help", "HELP"));
			HelpSubMenu->OnConfirmMenuItem.BindStatic([](){ FSlateApplication::Get().ShowSystemHelp(); });
		}

		MenuHelper::AddExistingMenuItem(RootMenuItem, MainMenuItem.ToSharedRef());
				
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FShooterIngameMenu::OnUIQuit);
#endif

		GameMenuWidget->MainMenu = GameMenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		GameMenuWidget->OnMenuHidden.BindSP(this,&FShooterIngameMenu::DetachGameMenu);
		GameMenuWidget->OnToggleMenu.BindSP(this,&FShooterIngameMenu::ToggleGameMenu);
		GameMenuWidget->OnGoBack.BindSP(this, &FShooterIngameMenu::OnMenuGoBack);
	}
}

void FShooterIngameMenu::CloseSubMenu()
{
	GameMenuWidget->MenuGoBack();
}

void FShooterIngameMenu::OnMenuGoBack(MenuPtr Menu)
{
	// if we are going back from options menu
	if (ShooterOptions->OptionsItem->SubMenu == Menu)
	{
		ShooterOptions->RevertChanges();
	}
}

bool FShooterIngameMenu::GetIsGameMenuUp() const
{
	return bIsGameMenuUp;
}

void FShooterIngameMenu::UpdateMenuOwner()
{
	if (GameMenuWidget.IsValid())
	{	
		GameMenuWidget->UpdateMenuOwner();
	}

	OwnerUserIndex = 0;
	if (PCOwner != nullptr && PCOwner->IsLocalPlayerController() && PCOwner->Player != nullptr)
	{
		UPlayer * Player = PCOwner->Player;		
		ULocalPlayer * LocalPlayer = CastChecked<ULocalPlayer>(Player);
		OwnerUserIndex = LocalPlayer->ControllerId;		
	}
}

void FShooterIngameMenu::DetachGameMenu()
{
	GEngine->GameViewport->RemoveViewportWidgetContent(GameMenuContainer.ToSharedRef());
	bIsGameMenuUp = false;
	// If the game is over enable the scoreboard
	if( PCOwner != NULL )
	{
		PCOwner->SetPause( false );
		AShooterHUD* ShooterHUD = PCOwner->GetShooterHUD();
		if( ( ShooterHUD != NULL ) && ( ShooterHUD->IsMatchOver() == true ) && ( PCOwner->IsPrimaryPlayer() == true ) )
		{
			ShooterHUD->ShowScoreboard( true );
		}
	}
}

void FShooterIngameMenu::ToggleGameMenu()
{
	if (!GameMenuWidget.IsValid())
	{
		return;
	}

	if (bIsGameMenuUp && GameMenuWidget->CurrentMenu != RootMenuItem->SubMenu)
	{
		GameMenuWidget->MenuGoBack();
		return;
	}

	if (!bIsGameMenuUp)
	{
		// Hide the scoreboard
		if( PCOwner != NULL )
		{
			AShooterHUD* ShooterHUD = PCOwner->GetShooterHUD();
			if( ShooterHUD != NULL )
			{
				ShooterHUD->ShowScoreboard( false );
			}
		}

		GEngine->GameViewport->AddViewportWidgetContent(
			SAssignNew(GameMenuContainer,SWeakWidget)
			.PossiblyNullContent(GameMenuWidget.ToSharedRef())
			);

		ShooterOptions->UpdateOptions();
		GameMenuWidget->BuildAndShowMenu();
		bIsGameMenuUp = true;
		PCOwner->SetCinematicMode(bIsGameMenuUp,false,false,true,true);
		PCOwner->SetPause( bIsGameMenuUp );
	} 
	else
	{
		//Start hiding animation
		GameMenuWidget->HideMenu();
		//enable player controls during hide animation
		FSlateApplication::Get().SetFocusToGameViewport();
		PCOwner->SetCinematicMode(false,false,false,true,true);
	}
}

void FShooterIngameMenu::OnCancelExitToMain()
{
	CloseSubMenu();
}

void FShooterIngameMenu::OnConfirmExitToMain()
{
	// ShooterGameState or ShooterGameMode (Depending on client/server) will poll the King state and take appropriate
	// action to bring us back to main. IngameMenu thus doesn't need to know anything about any of it.	
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EMainMenu);
}

void FShooterIngameMenu::OnUIQuit()
{
	GameMenuWidget->LockControls(true);
	GameMenuWidget->HideMenu();

	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
	MenuHelper::PlaySoundAndCall(PCOwner->GetWorld(),MenuSounds.ExitGameSound,OwnerUserIndex,this,&FShooterIngameMenu::Quit);
}

void FShooterIngameMenu::Quit()
{
	PCOwner->ConsoleCommand("quit");
}


#undef LOCTEXT_NAMESPACE
