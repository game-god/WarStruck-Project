// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "Slate.h"
#include "ShooterGame.h"

class SShooterConfirmationDialog : public SCompoundWidget
{
public:
	/** The player that owns the dialog. */
	ULocalPlayer* LocalPlayer;

	/** The delegate for confirming */
	FOnClicked OnConfirm;

	/** The delegate for cancelling */
	FOnClicked OnCancel;

	SLATE_BEGIN_ARGS( SShooterConfirmationDialog )
	{}

	SLATE_ARGUMENT(ULocalPlayer*, LocalPlayer)
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ActiveWindow)

	SLATE_TEXT_ARGUMENT(MessageText)
	SLATE_TEXT_ARGUMENT(ConfirmText)
	SLATE_TEXT_ARGUMENT(CancelText)

	SLATE_ARGUMENT(FOnClicked, OnConfirmClicked)
	SLATE_ARGUMENT(FOnClicked, OnCancelClicked)

	SLATE_END_ARGS()	

	void Construct(const FArguments& InArgs);

	void Enable();
	void Disable();

	virtual bool SupportsKeyboardFocus() const OVERRIDE;
	virtual FReply OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent) OVERRIDE;
	virtual FReply OnControllerButtonPressed(const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent) OVERRIDE;

private:

	TSharedPtr<SWindow> ActiveWindow; 

};
