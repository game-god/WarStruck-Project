// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterWelcomeMenu.h"
#include "ShooterStyle.h"
#include "SShooterConfirmationDialog.h"
#include "ShooterGameKing.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

class SShooterWelcomeMenuWidget : public SCompoundWidget
{
	/** The menu that owns this widget. */
	FShooterWelcomeMenu* MenuOwner;

	/** Workaround for a bug, run initialization on the first Tick() call. */
	bool bHasRunFirstTick;

	/** Animate the text so that the screen isn't static, for console cert requirements. */
	FCurveSequence TextAnimation;

	/** The actual curve that animates the text. */
	FCurveHandle TextColorCurve;

	SLATE_BEGIN_ARGS( SShooterWelcomeMenuWidget )
	{}

	SLATE_ARGUMENT(FShooterWelcomeMenu*, MenuOwner)

	SLATE_END_ARGS()

	virtual bool SupportsKeyboardFocus() const OVERRIDE
	{
		return true;
	}

	/**
	 * Gets the text color based on the current state of the animation.
	 *
	 * @return The text color based on the current state of the animation.
	 */
	FSlateColor GetTextColor() const
	{
		return FSlateColor(FMath::Lerp(FLinearColor(0.0f,0.0f,0.0f,1.0f), FLinearColor(0.5f,0.5f,0.5f,1.0f), TextColorCurve.GetLerp()));
	}

	void Construct( const FArguments& InArgs )
	{
		bHasRunFirstTick = false;
		MenuOwner = InArgs._MenuOwner;
		
		TextAnimation = FCurveSequence();
		const float AnimDuration = 1.5f;
		TextColorCurve = TextAnimation.AddCurve(0, AnimDuration, ECurveEaseFunction::QuadInOut);

		ChildSlot
		[
			SNew(SBorder)
			.Padding(30.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[ 
				SNew( STextBlock )
#if PLATFORM_PS4
				.Text( LOCTEXT("PressStartPS4", "PRESS CROSS BUTTON TO PLAY" ) )
#else
				.Text( LOCTEXT("PressStartXboxOne", "PRESS A TO PLAY" ) )
#endif
				.ColorAndOpacity(this, &SShooterWelcomeMenuWidget::GetTextColor)
				.TextStyle( FShooterStyle::Get(), "ShooterGame.WelcomeScreen.WelcomeTextStyle" )
			]
		];
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) OVERRIDE
	{
		if(!bHasRunFirstTick)
		{
			FSlateApplication::Get().SetKeyboardFocus(AsShared());
			bHasRunFirstTick = true;
		}

		if(!TextAnimation.IsPlaying())
		{
			if(TextAnimation.IsAtEnd())
			{
				TextAnimation.PlayReverse();
			}
			else
			{
				TextAnimation.Play();
			}
		}
	}

	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent ) OVERRIDE
	{
		return FReply::Handled();
	}

	virtual FReply OnControllerButtonPressed( const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent ) OVERRIDE
	{
		const FKey Key = ControllerEvent.GetEffectingButton();
		if (Key == EKeys::Gamepad_FaceButton_Bottom)
		{
			bool bSkipToMainMenu = true;

			const auto OnlineSub = IOnlineSubsystem::Get();
			if(OnlineSub)
			{
				const auto IdentityInterface = OnlineSub->GetIdentityInterface();
				if(IdentityInterface.IsValid())
				{
					const auto LoginStatus = IdentityInterface->GetLoginStatus(ControllerEvent.GetUserIndex());
					if(LoginStatus == ELoginStatus::NotLoggedIn)
					{
						// Show the account picker.
						const auto ExternalUI = OnlineSub->GetExternalUIInterface();
						if(ExternalUI.IsValid())
						{
							ExternalUI->ShowLoginUI(ControllerEvent.GetUserIndex(), false, IOnlineExternalUI::FOnLoginUIClosedDelegate::CreateSP(MenuOwner, &FShooterWelcomeMenu::HandleLoginUIClosed));
							bSkipToMainMenu = false;
						}
					}
				}
			}

			if(bSkipToMainMenu)
			{
				// If we couldn't show the external login UI for any reason, or if the user is
				// already logged in, just advance to the main menu immediately.
				MenuOwner->SetControllerAndAdvanceToMainMenu(ControllerEvent.GetUserIndex());
			}

			return FReply::Handled();
		}

		return FReply::Unhandled();
	}

	virtual FReply OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent) OVERRIDE
	{
		return FReply::Handled().ReleaseMouseCapture().CaptureJoystick(SharedThis( this ), true);
	}
};

void FShooterWelcomeMenu::Construct(ULocalPlayer* InLocalPlayer, AShooterGame_Menu* Game)
{
	GameModeOwner = Game;
	LocalPlayer = InLocalPlayer;
	PendingControllerIndex = 0;

	ConfirmationWidget = SNew( SShooterConfirmationDialog )
		.LocalPlayer(LocalPlayer)
		.ActiveWindow(GEngine->GameViewport->GetWindow())
		.MessageText(FString("If you continue without signing in, your progress will not be saved."))
		.ConfirmText(FString("A - Continue"))
		.CancelText(FString("B - Back"))
		.OnConfirmClicked(FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnContinueWithoutSavingConfirm))
		.OnCancelClicked(FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnContinueWithoutSavingBack));

	MenuWidget = SNew( SShooterWelcomeMenuWidget )
		.MenuOwner(this);	
}

void FShooterWelcomeMenu::AddToGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());
		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	}
}

void FShooterWelcomeMenu::RemoveFromGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
	}
}

void FShooterWelcomeMenu::HandleLoginUIClosed(TSharedPtr<FUniqueNetId> UniqueId, const int ControllerIndex)
{
	PendingControllerIndex = ControllerIndex;

	if(UniqueId.IsValid())
	{
		SetControllerAndAdvanceToMainMenu(ControllerIndex);
	}
	else
	{
		// Show a warning that your progress won't be saved if you continue without logging in. 
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
			ConfirmationWidget->Enable();
			FSlateApplication::Get().SetKeyboardFocus(ConfirmationWidget);
		}
	}
}

void FShooterWelcomeMenu::SetControllerAndAdvanceToMainMenu(const int ControllerIndex)
{
	if(LocalPlayer)
	{
		LocalPlayer->SetControllerId(ControllerIndex);
	}	
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EMainMenu);	
}

FReply FShooterWelcomeMenu::OnContinueWithoutSavingConfirm()
{
	ConfirmationWidget->Disable();
	SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	return FReply::Handled();
}

FReply FShooterWelcomeMenu::OnContinueWithoutSavingBack()
{
	ConfirmationWidget->Disable();
	GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());
	FSlateApplication::Get().SetKeyboardFocus(MenuWidget);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
