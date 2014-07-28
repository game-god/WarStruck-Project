// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterStyle.h"
#include "SShooterSplitScreenLobbyWidget.h"
#include "ShooterMenuItemWidgetStyle.h"
#include "ShooterMenuWidgetStyle.h"

#define LOCTEXT_NAMESPACE "ShooterGame.SplitScreenLobby"

int32 GShooterSplitScreenMax = 2;
static FAutoConsoleVariableRef CVarShooterSplitScreenMax(
	TEXT("r.ShooterSplitScreenMax"),
	GShooterSplitScreenMax,
	TEXT("Maximum number of split screen players.\n")
	TEXT("This will set the number of slots available in the split screen lobby.\n")
	TEXT("Default is 2."),
	ECVF_Default
	);

void SShooterSplitScreenLobby::Construct( const FArguments& InArgs )
{
	OnLoginChangedDelegate.BindSP( this, &SShooterSplitScreenLobby::OnLoginChanged );	
	NumSupportedSlots = FMath::Clamp(GShooterSplitScreenMax, 1, MAX_POSSIBLE_SLOTS);

#if PLATFORM_XBOXONE
	PressToPlayText = LOCTEXT("PressToPlay", "Press A to play");
	PressToStartMatchText = LOCTEXT("PressToStart", "Press A To Start Match");	
#else
	PressToPlayText = LOCTEXT("PressToPlay", "Press cross button to play");
	PressToStartMatchText = LOCTEXT("PressToStart", "Press Cross Button To Start Match");
#endif	

	LocalPlayer = InArgs._LocalPlayer;
	const FShooterMenuStyle* ItemStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterMenuStyle>("DefaultShooterMenuStyle");
	const FSlateBrush* SlotBrush = &ItemStyle->LeftBackgroundBrush;

	const FButtonStyle* ButtonStyle = &FShooterStyle::Get().GetWidgetStyle<FButtonStyle>("DefaultShooterButtonStyle");
	FLinearColor MenuTitleTextColor =  FLinearColor(FColor(155,164,182));
	FLinearColor PressXToStartColor =  FLinearColor(FColor(0,255,0));
	
	MasterUserBack = InArgs._OnCancelClicked;
	MasterUserPlay = InArgs._OnPlayClicked;	

	const float PaddingBetweenSlots = 10.0f;
	const float SlotWidth = 565.0f;
	const float SlotHeight = 300.0f;
	ChildSlot
	[	
		//main container.  Just start us out centered on the whole screen.
		SNew(SOverlay)
		+SOverlay::Slot()
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		[
			//at maximum we need a 2x2 grid.  So we need two vertical slots, which will each contain 2 horizontal slots.
			SNew( SVerticalBox )
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PaddingBetweenSlots) //put some space in between the slots
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SBox)
				[
					SNew(SBorder)
					.Padding(50.f)
					.VAlign(VAlign_Center)
					.HAlign(HAlign_Center)
					.BorderImage(SlotBrush)
					.BorderBackgroundColor(this, &SShooterSplitScreenLobby::ErrorMessageSlateColor)
					.ColorAndOpacity(this, &SShooterSplitScreenLobby::ErrorMessageLinearColor)
					[
						SNew( STextBlock )
						.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
						.ColorAndOpacity(FLinearColor(.7f, 0.02f, 0.02f, 1.0f))
						.Text(FString(TEXT("A guest account cannot play without its sponsor!")))
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(PaddingBetweenSlots) //put some space in between the slots
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew( SHorizontalBox)
				+SHorizontalBox::Slot()				
				.Padding(PaddingBetweenSlots) //put some space in between the slots				
				[
					SAssignNew(UserSlots[0], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)						
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()												
							.Padding(0.0f)
							.VAlign(VAlign_Bottom)
							.HAlign(HAlign_Center)
							[
								//first slot needs to have some text to say 'press X to start match'. Only master user can start the match.
								SAssignNew(UserTextWidgets[0], STextBlock)
								.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
								.ColorAndOpacity(MenuTitleTextColor)
								.Text(PressToPlayText)														
							]
							+SVerticalBox::Slot()							
							.Padding(5.0f)
							.VAlign(VAlign_Bottom)
							.HAlign(HAlign_Center)					
							[							
								SNew(STextBlock)
								.TextStyle(FShooterStyle::Get(), "ShooterGame.SplitScreenLobby.StartMatchTextStyle")							
								.Text(PressToStartMatchText)
							]
						]					
					]
				]
				+SHorizontalBox::Slot()					
				.Padding(PaddingBetweenSlots)				
				[
					SAssignNew(UserSlots[1], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SAssignNew(UserTextWidgets[1], STextBlock)
							.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(PressToPlayText)
						]
					]
				]
			]
			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Padding(PaddingBetweenSlots)
			[
				SNew( SHorizontalBox)
				+SHorizontalBox::Slot()							
				.Padding(PaddingBetweenSlots)				
				[
					SAssignNew(UserSlots[2], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SAssignNew(UserTextWidgets[2], STextBlock)
							.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(PressToPlayText)
						]
					]
				]

				+SHorizontalBox::Slot()				
				.Padding(PaddingBetweenSlots)				
				[
					SAssignNew(UserSlots[3], SBox)
					.HeightOverride(SlotHeight)
					.WidthOverride(SlotWidth)
					[
						SNew(SBorder)
						.Padding(0.0f)
						.VAlign(VAlign_Center)
						.HAlign(HAlign_Center)
						.BorderImage(SlotBrush)
						.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f))
						[
							SAssignNew(UserTextWidgets[3], STextBlock)
							.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
							.ColorAndOpacity(MenuTitleTextColor)
							.Text(PressToPlayText)
						]
					]
				]	
			]			
		]
	];
	
	Clear();
}

void SShooterSplitScreenLobby::Clear()
{	
	UpdateLobbyOwner();
	NumSupportedSlots = FMath::Clamp(GShooterSplitScreenMax, 1, MAX_POSSIBLE_SLOTS);
	FMemory::MemSet(UserIndices, 0xFF);

	for (int i = 0; i < NumSupportedSlots; ++i)
	{
		UserTextWidgets[i]->SetText(PressToPlayText);
		UserSlots[i]->SetVisibility(EVisibility::Visible);
		Users[i] = nullptr;
	}
	for (int i = NumSupportedSlots; i < MAX_POSSIBLE_SLOTS; ++i)
	{
		UserSlots[i]->SetVisibility(EVisibility::Collapsed);
	}

	TSharedPtr<STextBlock> UserTextWidgets[4];

	//always auto ready the menu owner.
	ReadyPlayer(LobbyOwner, LobbyOwnerUserIndex, 0);
}

void SShooterSplitScreenLobby::ReadyPlayer(TSharedPtr<FUniqueNetId> User, int UserIndex, int32 OpenSlot)
{
	// At least on Xbox One, the system doesn't prevent the same account from signing in on multiple gamepads.
	// If the user is already ready in a different slot, bail out.
	for(int i = 0; i < NumSupportedSlots; ++i)
	{
		if(Users[i].IsValid() && User.IsValid() && (*Users[i] == *User))
		{
			return;
		}
	}

	check(!Users[OpenSlot].IsValid());
	Users[OpenSlot] = User;
	UserIndices[OpenSlot] = UserIndex;

	bool bFoundNickName = false;

	const auto OnlineSub = IOnlineSubsystem::Get();

	FString UserNickName;
	if(OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			UserNickName = IdentityInterface->GetPlayerNickname(UserIndex);
			bFoundNickName = UserNickName.Len() > 0;
		}
	}

	if (!bFoundNickName)
	{
		// we don't support two un-logged in players, so only having one constant name is fine for now,
		// and this matches what the player's name defaults to in-game as well
		UserNickName = FString::Printf(TEXT("Player"));
	}

	UserTextWidgets[OpenSlot]->SetText(UserNickName);
}

void SShooterSplitScreenLobby::UnreadyPlayer(TSharedPtr<FUniqueNetId> User)
{
	for (int i = 0; i < NumSupportedSlots; ++i)
	{
		if (Users[i].IsValid() && User.IsValid() && (*Users[i] == *User))
		{
			Users[i] = nullptr;
			UserIndices[i] = -1;
			UserTextWidgets[i]->SetText(PressToPlayText);
		}
	}
}

void SShooterSplitScreenLobby::UpdateLobbyOwner()
{
	LobbyOwner = nullptr;
	LobbyOwnerUserIndex = 0;
	if (LocalPlayer != nullptr && LocalPlayer->IsLocalPlayerController() && LocalPlayer->Player != nullptr)
	{
		UPlayer * Player = LocalPlayer->Player;		
		ULocalPlayer * LocalPlayer = CastChecked<ULocalPlayer>(Player);
		auto Identity = Online::GetIdentityInterface();
		if(Identity.IsValid())
		{
			LobbyOwnerUserIndex = LocalPlayer->ControllerId;
			LobbyOwner = Identity->GetUniquePlayerId(LobbyOwnerUserIndex);
		}
	}
}

bool SShooterSplitScreenLobby::ConfirmSponsorsSatisfied() const
{
	// Will need more sophisticated logic if there are more than 2 players.
	check(NumSupportedSlots == 2);
	const auto OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			for(int i = 0; i < NumSupportedSlots; ++i)
			{
				if(Users[i].IsValid())
				{
					TSharedPtr<FUniqueNetId> Sponsor = IdentityInterface->GetSponsorUniquePlayerId(UserIndices[i]);
					if(Sponsor.IsValid())
					{
						// ONLY WORKS FOR 2 PLAYERS
						TSharedPtr<FUniqueNetId> Other = Users[(i + 1) % 2];
						return Other.IsValid() && (*Sponsor == *Other);
					}
				}
			}
		}
	}

	return true;
}

FSlateColor SShooterSplitScreenLobby::ErrorMessageSlateColor() const
{
	return ErrorMessageLinearColor();
}

FLinearColor SShooterSplitScreenLobby::ErrorMessageLinearColor() const
{
	static const double FullSeconds = 2.0;
	static const double FadeSeconds = 1.0;

	double CurrentTime = FPlatformTime::Seconds();
	double Delta = CurrentTime - TimeOfError;
	if(Delta <= FullSeconds)
	{
		return FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	else if(Delta > FullSeconds && Delta <= FullSeconds + FadeSeconds)
	{
		return FLinearColor(1.0f, 1.0f, 1.0f, 1.0 - ((Delta - FullSeconds) / FadeSeconds));
	}
	else
	{
		return FLinearColor(1.0f, 1.0f, 1.0f, 0);
	}
}

int32 SShooterSplitScreenLobby::GetNumSupportedSlots() const
{
	return NumSupportedSlots;
}

int32 SShooterSplitScreenLobby::GetNumReadyPlayers() const
{
	int32 NumReady = 0;
	for (int i = 0; i < NumSupportedSlots; ++i)
	{
		// Lobby owner is always ready, other users are ready if they have a valid user.
		if ((UserIndices[i] == LobbyOwnerUserIndex) || Users[i].IsValid())
		{
			++NumReady;
		}
	}
	return NumReady;
}

int32 SShooterSplitScreenLobby::GetUserIndexForSlot(int32 LobbySlot) const
{	
	check(LobbySlot < NumSupportedSlots);
	check(LobbySlot >= 0);
	return UserIndices[LobbySlot];
}

void SShooterSplitScreenLobby::OnLoginChanged(int32 LocalUserNum)
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			ELoginStatus::Type LoginStatus = IdentityInterface->GetLoginStatus(LocalUserNum);
			if (LoginStatus == ELoginStatus::NotLoggedIn)
			{
				UnreadyPlayer(IdentityInterface->GetUniquePlayerId(LocalUserNum));
			}
		}
	}
}

FReply SShooterSplitScreenLobby::OnControllerButtonPressed( const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent )
{
	const FKey Key = ControllerEvent.GetEffectingButton();
	const int32 UserIndex = ControllerEvent.GetUserIndex();			

	auto Identity = Online::GetIdentityInterface();
	if(!Identity.IsValid())
	{
		return FReply::Unhandled();
	}

	TSharedPtr<FUniqueNetId> EventUser = Identity->GetUniquePlayerId(UserIndex);

	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		if (UserIndex == LobbyOwnerUserIndex && GetNumReadyPlayers() >= 1)
		{
			if(ConfirmSponsorsSatisfied())
			{
				return MasterUserPlay.Execute();
			}
			else
			{
				TimeOfError = FPlatformTime::Seconds();

				return FReply::Handled();
			}
		}

		bool bAlreadyReady = false;
		for (int i = 0; i < NumSupportedSlots; ++i)
		{
			if (EventUser.IsValid() && (EventUser == Users[i]))
			{
				bAlreadyReady = true;
				break;
			}
		}

		if (!bAlreadyReady)
		{		
			int32 OpenSlot = -1;
			for (int i = 0; i < NumSupportedSlots; ++i)
			{
				if (!Users[i].IsValid() && UserIndices[i] == -1)
				{
					OpenSlot = i;
					break;
				}
			}
			if (OpenSlot != -1)
			{
				if(Identity.IsValid())
				{
					const auto LoginStatus = Identity->GetLoginStatus(UserIndex);
						
					if(LoginStatus == ELoginStatus::NotLoggedIn)
					{
						// Show the account picker.
						const auto ExternalUI = Online::GetExternalUIInterface();
						if(ExternalUI.IsValid())
						{
							ExternalUI->ShowLoginUI(UserIndex, false, IOnlineExternalUI::FOnLoginUIClosedDelegate::CreateSP(this, &SShooterSplitScreenLobby::HandleLoginUIClosedAndReady, OpenSlot));						
						}
					}
					else if(LoginStatus == ELoginStatus::LoggedIn || LoginStatus == ELoginStatus::UsingLocalProfile)
					{
						ReadyPlayer(EventUser, UserIndex, OpenSlot);
					}
				}		
				else
				{
					ReadyPlayer(EventUser, UserIndex, OpenSlot);
				}
			}
		}

		return FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Global_Back)
	{
		if (UserIndex == LobbyOwnerUserIndex)
		{
			return MasterUserBack.Execute();
		}
		else
		{
			UnreadyPlayer(EventUser);
		}
	}

	return FReply::Unhandled();
}


FReply SShooterSplitScreenLobby::OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent)
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			IdentityInterface->AddOnLoginChangedDelegate(OnLoginChangedDelegate);
		}
	}
	return FReply::Handled().ReleaseMouseCapture().CaptureJoystick(SharedThis( this ), true);
}

void SShooterSplitScreenLobby::OnKeyboardFocusLost( const FKeyboardFocusEvent& InKeyboardFocusEvent )
{
	const auto OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		const auto IdentityInterface = OnlineSub->GetIdentityInterface();
		if(IdentityInterface.IsValid())
		{
			IdentityInterface->ClearOnLoginChangedDelegate(OnLoginChangedDelegate);
		}
	}
}

void SShooterSplitScreenLobby::HandleLoginUIClosedAndReady( TSharedPtr<FUniqueNetId> UniqueId, const int UserIndex, const int SlotToJoin )
{
	// If a player signed in, UniqueId will be valid, and we can place him in the open slot.
	if(UniqueId.IsValid())
	{
		ReadyPlayer(UniqueId, UserIndex, SlotToJoin);
	}
}

#undef LOCTEXT_NAMESPACE