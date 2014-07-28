// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterMenuWidget.h"
#include "ShooterMenuItem.h"
#include "SShooterMenuItem.h"
#include "ShooterStyle.h"
#include "ShooterMenuWidgetStyle.h"

#define LOCTEXT_NAMESPACE "SShooterMenuWidget"


void SShooterMenuWidget::Construct(const FArguments& InArgs)
{
	MenuStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterMenuStyle>("DefaultShooterMenuStyle");

	bControlsLocked = false;
	bConsoleVisible = false;
	OutlineWidth = 20.0f;
	SelectedIndex = 0;
	PCOwner = InArgs._PCOwner;
	bGameMenu = InArgs._IsGameMenu;
	GameModeOwner = InArgs._GameModeOwner;
	ControllerHideMenuKey = EKeys::Gamepad_Special_Right;
	Visibility.Bind(this, &SShooterMenuWidget::GetSlateVisibility);
	FLinearColor MenuTitleTextColor =  FLinearColor(FColor(155,164,182));
	MenuHeaderHeight = 62.0f;
	MenuHeaderWidth = 287.0f;

	UpdateMenuOwner();

	ChildSlot
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Top)
			.Padding(TAttribute<FMargin>(this,&SShooterMenuWidget::GetMenuOffset))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SBox)
						.WidthOverride(MenuHeaderWidth)
						.HeightOverride(MenuHeaderHeight)
						[
							SNew(SImage)
							.ColorAndOpacity(this, &SShooterMenuWidget::GetHeaderColor)
							.Image(&MenuStyle->HeaderBackgroundBrush)
						]
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Left)
					.VAlign(VAlign_Fill)
					[
						SNew(SBox)
						.WidthOverride(MenuHeaderWidth)
						.HeightOverride(MenuHeaderHeight)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						[
							SNew(STextBlock)
							.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(this,&SShooterMenuWidget::GetMenuTitle)
						]
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
						SNew(SBorder)
						.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
						.ColorAndOpacity(this, &SShooterMenuWidget::GetBottomColor)
						.VAlign(VAlign_Top)
						.HAlign(HAlign_Left)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(TAttribute<FMargin>(this,&SShooterMenuWidget::GetLeftMenuOffset))
								[
									SNew(SBorder)
									.BorderImage(&MenuStyle->LeftBackgroundBrush)
									.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
									.Padding(FMargin(OutlineWidth))
									.DesiredSizeScale(this, &SShooterMenuWidget::GetBottomScale)
									.VAlign(VAlign_Top)
									.HAlign(HAlign_Left)
									[
										SAssignNew(LeftBox, SVerticalBox)
									]
								]
							]
							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.Padding(TAttribute<FMargin>(this,&SShooterMenuWidget::GetSubMenuOffset))
								.AutoHeight()
								[
									SNew(SBorder)
									.BorderImage(&MenuStyle->RightBackgroundBrush)
									.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
									.Padding(FMargin(OutlineWidth))
									.DesiredSizeScale(this, &SShooterMenuWidget::GetBottomScale)
									.VAlign(VAlign_Top)
									.HAlign(HAlign_Left)
									[
										SAssignNew(RightBox, SVerticalBox)
									]
								]
							]
						]
					
				]
			]
			
		]
	];
}

EVisibility SShooterMenuWidget::GetSlateVisibility() const
{
	return bConsoleVisible ? EVisibility::HitTestInvisible : EVisibility::Visible;
}

FString SShooterMenuWidget::GetMenuTitle() const 
{
	return CurrentMenuTitle.ToString();
}

void SShooterMenuWidget::LockControls(bool bEnable)
{
	bControlsLocked = bEnable;
}

void SShooterMenuWidget::UpdateMenuOwner()
{
	OwnerUserIndex = 0;
	if (PCOwner != nullptr && PCOwner->IsLocalPlayerController() && PCOwner->Player != nullptr)
	{
		UPlayer * Player = PCOwner->Player;		
		ULocalPlayer * LocalPlayer = CastChecked<ULocalPlayer>(Player);
		OwnerUserIndex = LocalPlayer->ControllerId;		
	}
}

void SShooterMenuWidget::BuildAndShowMenu()
{
	//grab the user settings
	UShooterGameUserSettings* UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	ScreenRes = UserSettings->GetScreenResolution();

	//Build left menu panel
	bLeftMenuChanging = false;
	bGoingBack = false;
	BuildLeftPanel(bGoingBack);

	//sets up whole main menu animations and launches them
	SetupAnimations();

	// Set up right side and launch animation if there is any submenu
	if (CurrentMenu.Num() > 0 && CurrentMenu.IsValidIndex(SelectedIndex) && CurrentMenu[SelectedIndex]->bVisible)
	{
		NextMenu = CurrentMenu[SelectedIndex]->SubMenu;
		if (NextMenu.Num() > 0)
		{
			BuildRightPanel();
			bSubMenuChanging = true;
		}
	}

	bMenuHiding = false;
	FSlateApplication::Get().PlaySound(MenuStyle->MenuEnterSound, OwnerUserIndex);
}

void SShooterMenuWidget::HideMenu()
{
	if (!bMenuHiding)
	{
		if(MenuWidgetAnimation.IsAtEnd())
		{
			MenuWidgetAnimation.PlayReverse();
		}
		else
		{
			MenuWidgetAnimation.Reverse();
		}
		bMenuHiding = true;
	}
}


void SShooterMenuWidget::SetupAnimations()
{
	//Setup a curve
	const float StartDelay = 0.0f;
	const float SecondDelay = bGameMenu ? 0.f : 0.3f;
	const float AnimDuration = 0.5f;
	const float MenuChangeDuration = 0.2f;

	//always animate the menu from the same side of the screen; it looks silly when it disappears to one place and reappears from another
	AnimNumber = 1;

	MenuWidgetAnimation = FCurveSequence();
	SubMenuWidgetAnimation = FCurveSequence();
	SubMenuScrollOutCurve = SubMenuWidgetAnimation.AddCurve(0,MenuChangeDuration,ECurveEaseFunction::QuadInOut);

	MenuWidgetAnimation = FCurveSequence();
	LeftMenuWidgetAnimation = FCurveSequence();
	LeftMenuScrollOutCurve = LeftMenuWidgetAnimation.AddCurve(0,MenuChangeDuration,ECurveEaseFunction::QuadInOut);
	LeftMenuWidgetAnimation.Play();

	//Define the fade in animation
	TopColorCurve = MenuWidgetAnimation.AddCurve(StartDelay, AnimDuration, ECurveEaseFunction::QuadInOut);

	//now, we want these to animate some time later

	//rolling out
	BottomScaleYCurve = MenuWidgetAnimation.AddCurve(StartDelay+SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
	//fading in
	BottomColorCurve = MenuWidgetAnimation.AddCurve(StartDelay+SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
	//moving from left side off screen
	ButtonsPosXCurve = MenuWidgetAnimation.AddCurve(StartDelay+SecondDelay, AnimDuration, ECurveEaseFunction::QuadInOut);
}

void SShooterMenuWidget::BuildLeftPanel(bool bGoingBack)
{
	if (CurrentMenu.Num() == 0)
	{
		//do not build anything if we do not have any active menu
		return;
	}
	LeftBox->ClearChildren();
	int32 PreviousIndex = -1;
	if (bLeftMenuChanging)
	{
		if (bGoingBack && MenuHistory.Num() > 0)
		{
			FShooterMenuInfo MenuInfo = MenuHistory.Pop();
			CurrentMenu = MenuInfo.Menu;
			CurrentMenuTitle = MenuInfo.MenuTitle;
			PreviousIndex = MenuInfo.SelectedIndex;
			if (CurrentMenu.Num() > 0 && CurrentMenu[PreviousIndex]->SubMenu.Num() > 0)
			{
				NextMenu = CurrentMenu[PreviousIndex]->SubMenu;
				bSubMenuChanging = true;
			}
		} 
		else if (PendingLeftMenu.Num() > 0)
		{
			MenuHistory.Push(FShooterMenuInfo(CurrentMenu, SelectedIndex, CurrentMenuTitle));
			CurrentMenuTitle = CurrentMenu[SelectedIndex]->GetText();
			CurrentMenu = PendingLeftMenu;
		}
	}
	SelectedIndex = PreviousIndex;
	//Setup the buttons
	for(int32 i = 0; i < CurrentMenu.Num(); ++i)
	{
		if (CurrentMenu[i]->bVisible)
		{
			TSharedPtr<SWidget> TmpWidget;
			if (CurrentMenu[i]->MenuItemType == EShooterMenuItemType::Standard)
			{
				TmpWidget = SAssignNew(CurrentMenu[i]->Widget, SShooterMenuItem)
					.PCOwner(PCOwner)
					.OnClicked(this, &SShooterMenuWidget::ButtonClicked, i)
					.Text(CurrentMenu[i]->GetText().ToString())
					.bIsMultichoice(false);
			} 
			else if (CurrentMenu[i]->MenuItemType == EShooterMenuItemType::MultiChoice)
			{
				TmpWidget = SAssignNew(CurrentMenu[i]->Widget, SShooterMenuItem)
					.PCOwner(PCOwner)
					.OnClicked(this, &SShooterMenuWidget::ButtonClicked, i)
					.Text(CurrentMenu[i]->GetText().ToString() )
					.bIsMultichoice(true)
					.OnArrowPressed(this, &SShooterMenuWidget::ChangeOption)
					.OptionText(this, &SShooterMenuWidget::GetOptionText, CurrentMenu[i]);
				UpdateArrows(CurrentMenu[i]);
			}
			else if (CurrentMenu[i]->MenuItemType == EShooterMenuItemType::CustomWidget)
			{
				TmpWidget = CurrentMenu[i]->CustomWidget;
			}
			if (TmpWidget.IsValid())
			{
				//set first selection to first valid widget
				if (SelectedIndex == -1)
				{
					SelectedIndex = i;
				}
				LeftBox->AddSlot()	.HAlign(HAlign_Left)	.AutoHeight()
				[
					TmpWidget.ToSharedRef()
				];
			}
		}
	}
	
	
	TSharedPtr<FShooterMenuItem> FirstMenuItem = CurrentMenu.IsValidIndex(SelectedIndex) ? CurrentMenu[SelectedIndex] : NULL;
	if (FirstMenuItem.IsValid() && FirstMenuItem->MenuItemType != EShooterMenuItemType::CustomWidget)
	{
		FirstMenuItem->Widget->SetMenuItemActive(true);
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
}

FString SShooterMenuWidget::GetOptionText(TSharedPtr<FShooterMenuItem> MenuItem) const
{
	FString Result;
	if (MenuItem->SelectedMultiChoice > -1 && MenuItem->SelectedMultiChoice < MenuItem->MultiChoice.Num())
	{
		Result = MenuItem->MultiChoice[MenuItem->SelectedMultiChoice].ToString();
	}
	return Result;
}

void SShooterMenuWidget::BuildRightPanel()
{
	RightBox->ClearChildren();
	
	if (NextMenu.Num() == 0) return;

	for(int32 i = 0; i < NextMenu.Num(); ++i)
	{
		if (NextMenu[i]->bVisible)
		{
			TSharedPtr<SShooterMenuItem> TmpButton;
			//Custom menu items are not supported in the right panel
			if (NextMenu[i]->MenuItemType == EShooterMenuItemType::Standard)
			{
				TmpButton = SAssignNew(NextMenu[i]->Widget, SShooterMenuItem)
					.PCOwner(PCOwner)
					.Text(NextMenu[i]->GetText().ToString())
					.InactiveTextAlpha(0.3f)
					.bIsMultichoice(false);
			}
			else if (NextMenu[i]->MenuItemType == EShooterMenuItemType::MultiChoice)
			{
				TmpButton = SAssignNew(NextMenu[i]->Widget, SShooterMenuItem)
					.PCOwner(PCOwner)
					.Text(NextMenu[i]->GetText().ToString() )
					.InactiveTextAlpha(0.3f)
					.bIsMultichoice(true)
					.OptionText(this, &SShooterMenuWidget::GetOptionText, NextMenu[i]);
			}
			RightBox->AddSlot()
			.HAlign(HAlign_Center)
			.AutoHeight()
			[
				TmpButton.ToSharedRef()
			];
		}
	}
}

void SShooterMenuWidget::UpdateArrows(TSharedPtr<FShooterMenuItem> MenuItem)
{
	const int32 MinIndex = MenuItem->MinMultiChoiceIndex > -1 ? MenuItem->MinMultiChoiceIndex : 0;
	const int32 MaxIndex = MenuItem->MaxMultiChoiceIndex > -1 ? MenuItem->MaxMultiChoiceIndex : MenuItem->MultiChoice.Num()-1;
	const int32 CurIndex = MenuItem->SelectedMultiChoice;
	if (CurIndex > MinIndex)
	{
		MenuItem->Widget->LeftArrowVisible = EVisibility::Visible;
	} 
	else
	{
		MenuItem->Widget->LeftArrowVisible = EVisibility::Collapsed;
	}
	if (CurIndex < MaxIndex)
	{
		MenuItem->Widget->RightArrowVisible = EVisibility::Visible;
	}
	else
	{
		MenuItem->Widget->RightArrowVisible = EVisibility::Collapsed;
	}
}

void SShooterMenuWidget::EnterSubMenu()
{
	bLeftMenuChanging = true;
	bGoingBack = false;
	FSlateApplication::Get().PlaySound(MenuStyle->MenuEnterSound, OwnerUserIndex);
}

void SShooterMenuWidget::MenuGoBack(bool bSilent)
{
	if (MenuHistory.Num() > 0)
	{
		if (!bSilent)
		{
			FSlateApplication::Get().PlaySound(MenuStyle->MenuBackSound, OwnerUserIndex);
		}
		bLeftMenuChanging = true;
		bGoingBack = true;
		OnGoBack.ExecuteIfBound(CurrentMenu);
	}
	else if (bGameMenu) // only when it's in-game menu variant
	{
		if (!bSilent)
		{
			FSlateApplication::Get().PlaySound(MenuStyle->MenuBackSound, OwnerUserIndex);
		}
		OnToggleMenu.ExecuteIfBound();
	}
	else
	{
#if SHOOTER_CONSOLE_UI
		// Go back to the welcome screen.
		HideMenu();
#endif
	}
}

void SShooterMenuWidget::ConfirmMenuItem()
{
	if (CurrentMenu[SelectedIndex]->OnConfirmMenuItem.IsBound())
	{
		CurrentMenu[SelectedIndex]->OnConfirmMenuItem.Execute();
	} 
	else if (CurrentMenu[SelectedIndex]->SubMenu.Num() > 0)
	{
		EnterSubMenu();
	}
}

void SShooterMenuWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	//Always tick the super
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	//ugly code seeing if the console is open
	UConsole* ViewportConsole = (GEngine !=NULL && GEngine->GameViewport != NULL) ? GEngine->GameViewport->ViewportConsole : NULL;
	if (ViewportConsole != NULL && (ViewportConsole->ConsoleState == "Typing" || ViewportConsole->ConsoleState == "Open"))
	{
		if (!bConsoleVisible)
		{
			bConsoleVisible = true;
			FSlateApplication::Get().SetFocusToGameViewport();
		}
	}
	else
	{
		if (bConsoleVisible)
		{
			bConsoleVisible = false;
			FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
		}
	}

	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportFrame)
	{
		FViewport* Viewport = GEngine->GameViewport->ViewportFrame->GetViewport();
		if (Viewport)
		{
			ScreenRes = Viewport->GetSizeXY();
		}
	}

	if (MenuWidgetAnimation.IsAtStart() && !bMenuHiding)
	{
		//Start the menu widget animation, set keyboard focus
		FadeIn();
	}
	else if (MenuWidgetAnimation.IsAtStart() && bMenuHiding)
	{
		bMenuHiding = false;
		//Send event, so menu can be removed
		OnMenuHidden.ExecuteIfBound();
	}

	if (MenuWidgetAnimation.IsAtEnd())
	{
		if (bLeftMenuChanging)
		{
			if (LeftMenuWidgetAnimation.IsAtEnd())
			{
				PendingLeftMenu = NextMenu;
				if (NextMenu.Num() > 0
					&& NextMenu.Top()->SubMenu.Num() > 0)
				{
					NextMenu = NextMenu.Top()->SubMenu;
				} 
				else 
				{
					NextMenu.Reset();
				}
				bSubMenuChanging = true;

				LeftMenuWidgetAnimation.PlayReverse();
			}
			if (!LeftMenuWidgetAnimation.IsPlaying())
			{
				if (CurrentMenu.Num() > 0)
				{
					BuildLeftPanel(bGoingBack);
					LeftMenuWidgetAnimation.Play();
				}
				//Focus the custom widget
				if (CurrentMenu.Num() == 1 && CurrentMenu.Top()->MenuItemType == EShooterMenuItemType::CustomWidget)
				{
					FSlateApplication::Get().SetKeyboardFocus(CurrentMenu.Top()->CustomWidget);
				}
				bLeftMenuChanging = false;
				RightBox->ClearChildren();
			}
		}
		if (bSubMenuChanging)
		{
			if (SubMenuWidgetAnimation.IsAtEnd())
			{
				SubMenuWidgetAnimation.PlayReverse();
			}
			if (!SubMenuWidgetAnimation.IsPlaying())
			{
				if (NextMenu.Num() > 0)
				{
					BuildRightPanel();
					SubMenuWidgetAnimation.Play();
				}
				bSubMenuChanging = false;
			}
		}
	}
}

FMargin SShooterMenuWidget::GetMenuOffset() const
{
	const float WidgetWidth = LeftBox->GetDesiredSize().X + RightBox->GetDesiredSize().X;
	const float WidgetHeight = LeftBox->GetDesiredSize().Y + MenuHeaderHeight;
	const float OffsetX = (ScreenRes.X - WidgetWidth - OutlineWidth*2)/2;
	const float AnimProgress = ButtonsPosXCurve.GetLerp();
	FMargin Result;

	switch (AnimNumber)
	{
		case 0:
			Result = FMargin(OffsetX + ScreenRes.X - AnimProgress*ScreenRes.X, (ScreenRes.Y - WidgetHeight)/2, 0, 0);
		break;
		case 1:
			Result = FMargin(OffsetX - ScreenRes.X + AnimProgress*ScreenRes.X, (ScreenRes.Y - WidgetHeight)/2, 0, 0);
		break;
		case 2:
			Result = FMargin(OffsetX, (ScreenRes.Y - WidgetHeight)/2 + ScreenRes.Y - AnimProgress*ScreenRes.Y, 0, 0);
		break;
		case 3:
			Result = FMargin(OffsetX, (ScreenRes.Y - WidgetHeight)/2 + -ScreenRes.Y + AnimProgress*ScreenRes.Y, 0, 0);
		break;
	}
	return Result;
}

FMargin SShooterMenuWidget::GetLeftMenuOffset() const
{
	const float LeftBoxSizeX = LeftBox->GetDesiredSize().X + OutlineWidth * 2;
	return FMargin(0, 0,-LeftBoxSizeX + LeftMenuScrollOutCurve.GetLerp() * LeftBoxSizeX,0);
}

FMargin SShooterMenuWidget::GetSubMenuOffset() const
{
	const float RightBoxSizeX = RightBox->GetDesiredSize().X + OutlineWidth * 2;
	return FMargin(0, 0,-RightBoxSizeX + SubMenuScrollOutCurve.GetLerp() * RightBoxSizeX,0);
}


FVector2D SShooterMenuWidget::GetBottomScale() const
{
	return FVector2D(BottomScaleYCurve.GetLerp(), BottomScaleYCurve.GetLerp());
}

FLinearColor SShooterMenuWidget::GetBottomColor() const
{
	return FMath::Lerp(FLinearColor(1,1,1,0), FLinearColor(1,1,1,1), BottomColorCurve.GetLerp());
}

FLinearColor SShooterMenuWidget::GetTopColor() const
{
	return FMath::Lerp(FLinearColor(1,1,1,0), FLinearColor(1,1,1,1), TopColorCurve.GetLerp());
}

FSlateColor SShooterMenuWidget::GetHeaderColor() const
{
	return CurrentMenuTitle.IsEmpty() ? FLinearColor::Transparent : FLinearColor::White;
}

FReply SShooterMenuWidget::ButtonClicked(int32 ButtonIndex)
{
	if (bControlsLocked)
	{
		return FReply::Handled();
	}

	if (SelectedIndex != ButtonIndex)
	{
		TSharedPtr<SShooterMenuItem> MenuItem = CurrentMenu[SelectedIndex]->Widget;
		MenuItem->SetMenuItemActive(false);
		SelectedIndex = ButtonIndex;
		MenuItem = CurrentMenu[SelectedIndex]->Widget;
		MenuItem->SetMenuItemActive(true);
		NextMenu = CurrentMenu[SelectedIndex]->SubMenu;
		bSubMenuChanging = true;
		FSlateApplication::Get().PlaySound(MenuStyle->MenuItemChangeSound, OwnerUserIndex);
	}
	else if (SelectedIndex == ButtonIndex)
	{
		ConfirmMenuItem();
	}

	return FReply::Handled().SetKeyboardFocus(SharedThis(this), EKeyboardFocusCause::SetDirectly);
}

void SShooterMenuWidget::FadeIn()
{
	//Start the menu widget playing
	MenuWidgetAnimation.Play();

	//Go into UI mode
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

FReply SShooterMenuWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	//If we clicked anywhere, jump to the end
	if(MenuWidgetAnimation.IsPlaying())
	{
		MenuWidgetAnimation.JumpToEnd();
	}

	//Set the keyboard focus 
	return FReply::Handled()
	.SetKeyboardFocus(SharedThis(this), EKeyboardFocusCause::SetDirectly);
}

void SShooterMenuWidget::ChangeOption(int32 MoveBy)
{
	TSharedPtr<FShooterMenuItem> MenuItem = CurrentMenu[SelectedIndex];

	const int32 MinIndex = MenuItem->MinMultiChoiceIndex > -1 ? MenuItem->MinMultiChoiceIndex : 0;
	const int32 MaxIndex = MenuItem->MaxMultiChoiceIndex > -1 ? MenuItem->MaxMultiChoiceIndex : MenuItem->MultiChoice.Num()-1;
	const int32 CurIndex = MenuItem->SelectedMultiChoice;

	if (MenuItem->MenuItemType == EShooterMenuItemType::MultiChoice)
	{
		if ( CurIndex + MoveBy >= MinIndex && CurIndex + MoveBy <= MaxIndex)
		{
			MenuItem->SelectedMultiChoice += MoveBy;
			MenuItem->OnOptionChanged.ExecuteIfBound(MenuItem, MenuItem->SelectedMultiChoice);
			FSlateApplication::Get().PlaySound(MenuStyle->OptionChangeSound, OwnerUserIndex);
		}
		UpdateArrows(MenuItem);
	}
}

int32 SShooterMenuWidget::GetNextValidIndex(int32 MoveBy)
{
	int32 Result = SelectedIndex;
	if (MoveBy != 0 && SelectedIndex + MoveBy > -1 && SelectedIndex+MoveBy < CurrentMenu.Num())
	{
		Result = SelectedIndex + MoveBy;
		//look for non-hidden menu item
		while (!CurrentMenu[Result]->Widget.IsValid())
		{
			MoveBy > 0 ? Result++ : Result--;
			//when moved outside of array, just return current selection
			if (!CurrentMenu.IsValidIndex(Result))
			{
				Result = SelectedIndex;
				break;
			}
		}
	}
	return Result;
}

FReply SShooterMenuWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent)
{
	FReply Result = FReply::Unhandled();
	if (!bControlsLocked)
	{
		const FKey Key = InKeyboardEvent.GetKey();
	
		if (Key == EKeys::Up || Key == EKeys::Down)
		{
			int32 MoveBy = Key == EKeys::Up ? -1 : 1;
			int32 NextValidIndex = GetNextValidIndex(MoveBy);
			if (NextValidIndex != SelectedIndex)
			{
				ButtonClicked(NextValidIndex);
			}
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Left || Key == EKeys::Right)
		{
			int32 MoveBy = Key == EKeys::Left ? -1 : 1;
			ChangeOption(MoveBy);
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Enter)
		{
			ConfirmMenuItem();
			Result = FReply::Handled();
		} 
		else if (Key == EKeys::Escape )
		{
			MenuGoBack();
			Result = FReply::Handled();
		}
	}
	return Result;
}

FReply SShooterMenuWidget::OnControllerButtonPressed( const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent )
{
	FReply Result = FReply::Unhandled();
	const int32 UserIndex = ControllerEvent.GetUserIndex();
	bool bEventUserCanInteract = OwnerUserIndex == -1 || UserIndex == OwnerUserIndex;

	if (!bControlsLocked && bEventUserCanInteract)
	{
		const FKey Key = ControllerEvent.GetEffectingButton();
		if (Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
		{
			int32 NextValidIndex = GetNextValidIndex(-1);
			if (NextValidIndex != SelectedIndex)
			{
				ButtonClicked(NextValidIndex);
			}
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
		{
			int32 NextValidIndex = GetNextValidIndex(1);
			if (NextValidIndex != SelectedIndex)
			{
				ButtonClicked(NextValidIndex);
			}
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_DPad_Left || Key == EKeys::Gamepad_LeftStick_Left)
		{
			ChangeOption(-1);
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_DPad_Right || Key == EKeys::Gamepad_LeftStick_Right)
		{
			ChangeOption(1);
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Gamepad_FaceButton_Bottom && !ControllerEvent.IsRepeat())
		{
			ConfirmMenuItem();
			Result = FReply::Handled();
		} 
		else if ((Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Left || Key == EKeys::Global_Back || Key == EKeys::Global_View) && !ControllerEvent.IsRepeat())
		{
			MenuGoBack();
			Result = FReply::Handled();
		} 
		else if ((Key == ControllerHideMenuKey || Key == EKeys::Global_Play || Key == EKeys::Global_Menu) && !ControllerEvent.IsRepeat())
		{
			OnToggleMenu.ExecuteIfBound();
			Result = FReply::Handled();
		}
	}
	return Result;
}

FReply SShooterMenuWidget::OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent)
{
	//Focus the custom widget
	if (CurrentMenu.Num() == 1 && CurrentMenu.Top()->MenuItemType == EShooterMenuItemType::CustomWidget)
	{
		return FReply::Handled().ReleaseJoystickCapture().SetKeyboardFocus(CurrentMenu.Top()->CustomWidget.ToSharedRef(),EKeyboardFocusCause::SetDirectly);
	}

	return FReply::Handled().ReleaseMouseCapture().CaptureJoystick(SharedThis( this ), true);
}

#undef LOCTEXT_NAMESPACE
