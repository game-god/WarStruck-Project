// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterStyle.h"
#include "SShooterConfirmationDialog.h"
#include "ShooterGameKing.h"
#include "ShooterMessageMenu.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterMessageMenu::Construct(ULocalPlayer* InLocalPlayer, AShooterGame_Menu* Game, const FString& Message, const FString& OKButtonText, const FString& CancelButtonText)
{
	GameModeOwner = Game;
	LocalPlayer = InLocalPlayer;
	PendingControllerIndex = 0;

	MenuWidget = SNew(SShooterConfirmationDialog).LocalPlayer(Cast<ULocalPlayer>(LocalPlayer))
		.ActiveWindow(GEngine->GameViewport->GetWindow())
		.MessageText(Message)
		.ConfirmText(OKButtonText)
		.CancelText(CancelButtonText)
		.OnConfirmClicked(FOnClicked::CreateRaw(this, &FShooterMessageMenu::OnClickedOK))
		.OnCancelClicked(FOnClicked::CreateRaw(this, &FShooterMessageMenu::OnClickedCancel));
	
}

void FShooterMessageMenu::AddToGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());
		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	}
}

void FShooterMessageMenu::RemoveFromGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
	}
}

void FShooterMessageMenu::SetControllerAndAdvanceToMainMenu(const int ControllerIndex)
{
	if(LocalPlayer)
	{
		LocalPlayer->SetControllerId(ControllerIndex);
	}
	
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EMainMenu);
}

FReply FShooterMessageMenu::OnClickedOK()
{
	OKButtonDelegate.ExecuteIfBound();
	MenuWidget->Disable();
	SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	return FReply::Handled();
}

FReply FShooterMessageMenu::OnClickedCancel()
{
	CancelButtonDelegate.ExecuteIfBound();
	MenuWidget->Disable();
	SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	return FReply::Handled();
}




void FShooterMessageMenu::SetOKClickedDelegate(FMessageMenuButtonClicked InButtonDelegate)
{
	OKButtonDelegate = InButtonDelegate;
}

void FShooterMessageMenu::SetCancelClickedDelegate(FMessageMenuButtonClicked InButtonDelegate)
{
	CancelButtonDelegate = InButtonDelegate;
}


#undef LOCTEXT_NAMESPACE
