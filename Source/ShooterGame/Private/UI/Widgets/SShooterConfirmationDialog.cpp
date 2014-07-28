// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterStyle.h"
#include "SShooterConfirmationDialog.h"
#include "ShooterMenuItemWidgetStyle.h"

void SShooterConfirmationDialog::Construct( const FArguments& InArgs )
{	
	ActiveWindow = InArgs._ActiveWindow;
	LocalPlayer = InArgs._LocalPlayer;

	OnConfirm = InArgs._OnConfirmClicked;
	OnCancel = InArgs._OnCancelClicked;

	const FShooterMenuItemStyle* ItemStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterMenuItemStyle>("DefaultShooterMenuItemStyle");
	const FButtonStyle* ButtonStyle = &FShooterStyle::Get().GetWidgetStyle<FButtonStyle>("DefaultShooterButtonStyle");
	FLinearColor MenuTitleTextColor =  FLinearColor(FColor(155,164,182));
	ChildSlot
	[					
		SNew( SVerticalBox )
		+SVerticalBox::Slot()
		.AutoHeight()
		.Padding(20.0f)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			SNew(SBorder)
			.Padding(100.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(&ItemStyle->BackgroundBrush)
			.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
			[
				SNew( STextBlock )
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
				.ColorAndOpacity(MenuTitleTextColor)
				.Text(InArgs._MessageText)
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.Padding(20.0f)
		[
			SNew( SHorizontalBox)
			+SHorizontalBox::Slot()			
			.AutoWidth()
			.Padding(20.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew( SButton )
				.ContentPadding(100)
				.OnClicked(InArgs._OnConfirmClicked)
				.Text(InArgs._ConfirmText)			
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
				.ButtonStyle(ButtonStyle)
			]

			+SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(20.0f)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew( SButton )
					.ContentPadding(100)
					.OnClicked(InArgs._OnCancelClicked)
					.Text(InArgs._CancelText)
					.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
					.ButtonStyle(ButtonStyle)
				]	
		]			
	];
}

void SShooterConfirmationDialog::Enable()
{
	if (ActiveWindow.IsValid())
	{
		SOverlay::FOverlaySlot& OverLaySlot = ActiveWindow->AddOverlaySlot();
		OverLaySlot.HAlign(HAlign_Center);
		OverLaySlot.VAlign(VAlign_Center);
		OverLaySlot.Widget = SharedThis(this);
	}
}

void SShooterConfirmationDialog::Disable()
{
	if (ActiveWindow.IsValid())
	{
		ActiveWindow->RemoveOverlaySlot(SharedThis(this));
	}
}

bool SShooterConfirmationDialog::SupportsKeyboardFocus() const
{
	return true;
}

FReply SShooterConfirmationDialog::OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent)
{
	return FReply::Handled().ReleaseMouseCapture().CaptureJoystick(SharedThis( this ), true);
}

FReply SShooterConfirmationDialog::OnControllerButtonPressed( const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent )
{
	const FKey Key = ControllerEvent.GetEffectingButton();
	const int32 UserIndex = ControllerEvent.GetUserIndex();			

	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		if(OnConfirm.IsBound())
		{
			return OnConfirm.Execute();
		}
	}
	else if (Key == EKeys::Gamepad_FaceButton_Right)
	{
		if(OnCancel.IsBound())
		{
			return OnCancel.Execute();
		}
	}

	return FReply::Unhandled();
}

