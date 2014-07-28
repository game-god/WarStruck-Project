// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterLeaderboard.h"
#include "ShooterStyle.h"

void SShooterLeaderboard::Construct(const FArguments& InArgs)
{
	PCOwner = InArgs._PCOwner;
	OwnerWidget = InArgs._OwnerWidget;
	const int32 BoxWidth = 125;
	bReadingStats = false;

	LeaderboardReadCompleteDelegate = FOnLeaderboardReadCompleteDelegate::CreateRaw(this, &SShooterLeaderboard::OnStatsRead);

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SBox)  
		.WidthOverride(600)
		.HeightOverride(600)
		[
			SAssignNew(RowListWidget, SListView< TSharedPtr<FLeaderboardRow> >)
			.ItemHeight(20)
			.ListItemsSource(&StatRows)
			.SelectionMode(ESelectionMode::Single)
			.OnGenerateRow(this, &SShooterLeaderboard::MakeListViewWidget)
			.OnSelectionChanged(this, &SShooterLeaderboard::EntrySelectionChanged)
			.OnMouseButtonDoubleClick(this,&SShooterLeaderboard::OnListItemDoubleClicked)
			.HeaderRow(
			SNew(SHeaderRow)
			+ SHeaderRow::Column("Rank").FixedWidth(BoxWidth/3) .DefaultLabel(NSLOCTEXT("LeaderBoard", "PlayerRankColumn", "Rank"))
			+ SHeaderRow::Column("PlayerName").FixedWidth(BoxWidth*2) .DefaultLabel(NSLOCTEXT("LeaderBoard", "PlayerNameColumn", "Player Name"))
			+ SHeaderRow::Column("Kills") .DefaultLabel(NSLOCTEXT("LeaderBoard", "KillsColumn", "Kills"))
			+ SHeaderRow::Column("Deaths") .DefaultLabel(NSLOCTEXT("LeaderBoard", "DeathsColumn", "Deaths")))
		]
	];
}

/** 
 * Get the current game
 * @return The current game
 */
AShooterGameMode* SShooterLeaderboard::GetGame() const
{
	const APlayerController* const Owner = PCOwner.Get();
	if (Owner != NULL)
	{
		UWorld* const World = Owner->GetWorld();
		if (World)
		{
			return World->GetAuthGameMode<AShooterGameMode>();
		}
	}

	return NULL;
}

/** 
 * Get the current game session
 * @return The current game session
 */
AShooterGameSession* SShooterLeaderboard::GetGameSession() const
{
	AShooterGameMode* const ShooterGameMode = GetGame();
	if (ShooterGameMode)
	{
		return Cast<AShooterGameSession>(ShooterGameMode->GameSession);
	}

	return NULL;
}

/** Starts reading leaderboards for the game */
void SShooterLeaderboard::ReadStats()
{
	StatRows.Reset();

	IOnlineSubsystem* const OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
		if (Leaderboards.IsValid())
		{
			ReadObject = MakeShareable(new FShooterAllTimeMatchResultsRead());
			FOnlineLeaderboardReadRef ReadObjectRef = ReadObject.ToSharedRef();

			Leaderboards->AddOnLeaderboardReadCompleteDelegate(LeaderboardReadCompleteDelegate);

			// We are about to read the stats. The delegate will set this to false once the read is complete.
			bReadingStats = true;
			if( Leaderboards->ReadLeaderboardsForFriends(0, ReadObjectRef) == false )
			{
				// If we got an error reading the board clear the reading stats flag
				bReadingStats = false;
			}
		}
		else
		{
			// TODO: message the user?
		}
	}
}

/** Called on a particular leaderboard read */
void SShooterLeaderboard::OnStatsRead(bool bWasSuccessful)
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if(OnlineSub)
	{
		IOnlineLeaderboardsPtr Leaderboards = OnlineSub->GetLeaderboardsInterface();
		if (Leaderboards.IsValid())
		{
			Leaderboards->ClearOnLeaderboardReadCompleteDelegate(LeaderboardReadCompleteDelegate);
		}
	}

	if (bWasSuccessful)
	{
		if (ReadObject.IsValid())
		{
			for (int Idx=0; Idx < ReadObject->Rows.Num(); ++Idx)
			{
				TSharedPtr<FLeaderboardRow> NewLeaderboardRow = MakeShareable(new FLeaderboardRow());
				NewLeaderboardRow->Rank = FString::FromInt(ReadObject->Rows[Idx].Rank);
				NewLeaderboardRow->PlayerName = ReadObject->Rows[Idx].NickName;

				FVariantData * Variant = ReadObject->Rows[Idx].Columns.Find(LEADERBOARD_STAT_KILLS);
				if (Variant)
				{
					int32 Val;
					Variant->GetValue(Val);
					NewLeaderboardRow->Kills = FString::FromInt(Val);
				}

				Variant = ReadObject->Rows[Idx].Columns.Find(LEADERBOARD_STAT_DEATHS);
				if (Variant)
				{
					int32 Val;
					Variant->GetValue(Val);
					NewLeaderboardRow->Deaths = FString::FromInt(Val);
				}

				StatRows.Add(NewLeaderboardRow);
			}

			RowListWidget->RequestListRefresh();

			ReadObject = NULL;
		}
	}

	bReadingStats = false;
}

void SShooterLeaderboard::OnKeyboardFocusLost( const FKeyboardFocusEvent& InKeyboardFocusEvent )
{
	if (InKeyboardFocusEvent.GetCause() != EKeyboardFocusCause::SetDirectly)
	{
		FSlateApplication::Get().SetKeyboardFocus(SharedThis( this ));
	}
}

FReply SShooterLeaderboard::OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent)
{
	return FReply::Handled().SetKeyboardFocus(RowListWidget.ToSharedRef(),EKeyboardFocusCause::SetDirectly).CaptureJoystick(SharedThis( this ));
}

void SShooterLeaderboard::EntrySelectionChanged(TSharedPtr<FLeaderboardRow> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedItem = InItem;
}

void SShooterLeaderboard::OnListItemDoubleClicked(TSharedPtr<FLeaderboardRow> InItem)
{
	SelectedItem = InItem;
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

void SShooterLeaderboard::MoveSelection(int32 MoveBy)
{
	int32 SelectedItemIndex = StatRows.IndexOfByKey(SelectedItem);

	if (SelectedItemIndex+MoveBy > -1 && SelectedItemIndex+MoveBy < StatRows.Num())
	{
		RowListWidget->SetSelection(StatRows[SelectedItemIndex+MoveBy]);
	}
}

FReply SShooterLeaderboard::OnControllerButtonPressed(const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent)
{
	FReply Result = FReply::Unhandled();
	const FKey Key = ControllerEvent.GetEffectingButton();

	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		// TODO: show details

		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	} 
	else if (Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
	{
		MoveSelection(-1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
	{
		MoveSelection(1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Gamepad_FaceButton_Right || Key == EKeys::Gamepad_Special_Left)
	{
		if (bReadingStats)
		{
			// block return if we're still reading the stats
			Result = FReply::Handled();
		}
	}
	return Result.IsEventHandled() ? Result : OwnerWidget->OnControllerButtonPressed(MyGeometry, ControllerEvent);
}

FReply SShooterLeaderboard::OnKeyDown(const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent) 
{
	FReply Result = FReply::Unhandled();
	const FKey Key = InKeyboardEvent.GetKey();
	if (Key == EKeys::Enter)
	{
		// TODO: 
		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
	else if (Key == EKeys::Escape)
	{
		if (bReadingStats)
		{
			// block return if we're still reading the stats
			Result = FReply::Handled();
		}
	}
	return Result;
}

TSharedRef<ITableRow> SShooterLeaderboard::MakeListViewWidget(TSharedPtr<FLeaderboardRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SLeaderboardRowWidget : public SMultiColumnTableRow< TSharedPtr<FLeaderboardRow> >
	{
	public:
		SLATE_BEGIN_ARGS(SLeaderboardRowWidget){}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FLeaderboardRow> InItem)
		{
			Item = InItem;
			SMultiColumnTableRow< TSharedPtr<FLeaderboardRow> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			FString ItemText;
			if (ColumnName == "Rank")
			{
				ItemText = Item->Rank;
			}
			else if (ColumnName == "PlayerName")
			{
				ItemText = Item->PlayerName.Left(MAX_PLAYER_NAME_LENGTH);
			}
			else if (ColumnName == "Kills")
			{
				ItemText = Item->Kills;
			}
			else if (ColumnName == "Deaths")
			{
				ItemText = Item->Deaths;
			}
			return SNew(STextBlock)
				.Text(ItemText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.ScoreboardListTextStyle");		
		}
		TSharedPtr<FLeaderboardRow> Item;
	};
	return SNew(SLeaderboardRowWidget, OwnerTable, Item);
}
