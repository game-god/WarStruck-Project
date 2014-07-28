// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterScoreboardWidget.h"
#include "ShooterStyle.h"
#include "ShooterScoreboardWidgetStyle.h"

#define LOCTEXT_NAMESPACE "ShooterScoreboard"

void SShooterScoreboardWidget::Construct(const FArguments& InArgs)
{
	ScoreboardStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterScoreboardStyle>("DefaultShooterScoreboardStyle");

	PCOwner = InArgs._PCOwner;
	ScoreboardTint = FLinearColor(0.0f,0.0f,0.0f,0.4f);
	ScoreBoxWidth = 140.0f;
	ScoreCountUpTime = 2.0f;

	ScoreboardStartTime = FPlatformTime::Seconds();
	MatchState = InArgs._MatchState.Get();

	UpdatePlayerStateMaps();
	
	Columns.Add(FColumnData(LOCTEXT("KillsColumn", "Kills").ToString(),
		ScoreboardStyle->KillStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Kills)));

	Columns.Add(FColumnData(LOCTEXT("DeathsColumn", "Deaths").ToString(),
		ScoreboardStyle->DeathStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Deaths)));

	Columns.Add(FColumnData(LOCTEXT("ScoreColumn", "Score").ToString(),
		ScoreboardStyle->ScoreStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Score)));

	TSharedPtr<SHorizontalBox> HeaderCols;

	const TSharedRef<SVerticalBox> ScoreboardGrid = SNew(SVerticalBox)
	// HEADER ROW			
	+SVerticalBox::Slot() .AutoHeight()
	[
		//Player Name autosized column
		SAssignNew(HeaderCols, SHorizontalBox)
		+SHorizontalBox::Slot() .Padding(5)
		[
			SNew(SBorder)
			.Padding(5)
			.VAlign(VAlign_Center)	
			.HAlign(HAlign_Center)
			.BorderImage(&ScoreboardStyle->ItemBorderBrush)
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PlayerNameColumn", "Player Name").ToString())
				.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
			]
		]
	];

	for (uint8 ColIdx = 0; ColIdx < Columns.Num(); ColIdx++ )
	{
		//Header constant sized columns
		HeaderCols->AddSlot() .VAlign(VAlign_Center) .HAlign(HAlign_Center) .AutoWidth() .Padding(5)
		[
			SNew(SBorder)
			.Padding(5)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(&ScoreboardStyle->ItemBorderBrush)
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(SBox)
				.WidthOverride(ScoreBoxWidth)
				.HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot() .AutoWidth() .VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Columns[ColIdx].Name)
						.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
						.ColorAndOpacity(Columns[ColIdx].Color)
					]
				]
			]
		];
	}

	ScoreboardGrid->AddSlot() .AutoHeight()
	[
		SAssignNew(ScoreboardData, SVerticalBox)
	];
	UpdateScoreboardGrid();

	SBorder::Construct(
		SBorder::FArguments()
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTint)
		[
			ScoreboardGrid
		]			
	);
}

FString SShooterScoreboardWidget::GetMatchRestartText() const
{
	if (PCOwner.IsValid() && (PCOwner->GetWorld() != NULL ))
	{
		AShooterGameState* const GameState = Cast<AShooterGameState>(PCOwner->GetWorld()->GameState);
		if (GameState)
		{
			if (GameState->RemainingTime > 0)
			{
				return FText::Format(LOCTEXT("MatchRestartTimeString", "New match begins in: {0}"), FText::AsNumber(GameState->RemainingTime)).ToString();
			}
			else
			{
				return LOCTEXT("MatchRestartingString", "Starting new match...").ToString();
			}
		}
	}

	return TEXT("");
}

FString SShooterScoreboardWidget::GetMatchOutcomeText() const
{
	FString OutcomeText;

	if (MatchState == EShooterMatchState::Won)
	{
		OutcomeText = LOCTEXT("Winner", "YOU ARE THE WINNER!").ToString();
	} 
	else if (MatchState == EShooterMatchState::Lost)
	{
		OutcomeText = LOCTEXT("Loser", "Match has finished").ToString();
	}

	return OutcomeText;
}

void SShooterScoreboardWidget::UpdateScoreboardGrid()
{
	ScoreboardData->ClearChildren();
	for (uint8 TeamNum = 0; TeamNum < PlayerStateMaps.Num(); TeamNum++)
	{
		//Player rows from each team
		ScoreboardData->AddSlot() .AutoHeight()
			[
				MakePlayerRows(TeamNum)
			];
		//If we have more than one team, we are playing team based game mode, add totals
		if (PlayerStateMaps.Num() > 1 && PlayerStateMaps[TeamNum].Num() > 0)
		{
			// Horizontal Ruler
			ScoreboardData->AddSlot() .AutoHeight() .Padding(5,0)
				[
					SNew(SBorder)
					.Padding(1)
					.BorderImage(&ScoreboardStyle->ItemBorderBrush)
				];
			ScoreboardData->AddSlot() .AutoHeight()
				[
					MakeTotalsRow(TeamNum)
				];
		}
	}

	if (MatchState > EShooterMatchState::Playing)
	{
		ScoreboardData->AddSlot() .AutoHeight() .Padding(5)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot() .HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
						.Text(this, &SShooterScoreboardWidget::GetMatchOutcomeText)
					]
				]
			];

		ScoreboardData->AddSlot() .AutoHeight() .Padding(5)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot() .HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
						.Text(this, &SShooterScoreboardWidget::GetMatchRestartText)
					]
				]
			];

	}
}

void SShooterScoreboardWidget::UpdatePlayerStateMaps()
{
	if (PCOwner.IsValid())
	{
		AShooterGameState* const GameState = Cast<AShooterGameState>(PCOwner->GetWorld()->GameState);
		if (GameState)
		{
			bool bRequiresWidgetUpdate = false;
			const int32 NumTeams = FMath::Max(GameState->NumTeams, 1);
			LastTeamPlayerCount.Reset();
			LastTeamPlayerCount.AddZeroed(PlayerStateMaps.Num());
			for (int32 i = 0; i < PlayerStateMaps.Num(); i++)
			{
				LastTeamPlayerCount[i] = PlayerStateMaps[i].Num();
			}

			PlayerStateMaps.Reset();
			PlayerStateMaps.AddZeroed(NumTeams);
		
			for (int32 i = 0; i < NumTeams; i++)
			{
				GameState->GetRankedMap(i, PlayerStateMaps[i]);

				if (LastTeamPlayerCount.Num() > 0 && PlayerStateMaps[i].Num() != LastTeamPlayerCount[i])
				{
					bRequiresWidgetUpdate = true;
				}
			}
			if (bRequiresWidgetUpdate)
			{
				UpdateScoreboardGrid();
			}
		}
	}
}

void SShooterScoreboardWidget::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	UpdatePlayerStateMaps();
}

EVisibility SShooterScoreboardWidget::PlayerPresenceToItemVisibility(uint8 TeamNum, int32 PlayerId) const
{
	return GetSortedPlayerState(TeamNum, PlayerId) ? EVisibility::Visible : EVisibility::Collapsed;
}

FSlateColor SShooterScoreboardWidget::GetScoreboardBorderColor(uint8 TeamNum, int32 PlayerId) const
{
	APlayerController* const PC = PCOwner.Get();
	const bool bIsMe = PC ? (GetSortedPlayerState(TeamNum, PlayerId) == PC->PlayerState) : false;

	const int32 RedTeam = 0;
	const float BaseValue = bIsMe == true ? 0.15f : 0.0f;
	const float AlphaValue = bIsMe == true ? 1.0f : 0.3f;
	float RedValue = TeamNum == RedTeam ? 0.25f : 0.0f;
	float BlueValue = TeamNum != RedTeam ? 0.25f : 0.0f;
	return FLinearColor(BaseValue + RedValue, BaseValue, BaseValue + BlueValue, AlphaValue);
}

FString SShooterScoreboardWidget::GetPlayerName(uint8 TeamNum, int32 PlayerId) const
{
	AShooterPlayerState* PlayerState = GetSortedPlayerState(TeamNum, PlayerId);
	if (PlayerState)
	{
		return PlayerState->GetShortPlayerName();
	}

	return TEXT("");
}

FString SShooterScoreboardWidget::GetStat(FOnGetPlayerStateAttribute Getter, uint8 TeamNum, int32 PlayerId) const
{
	int32 StatTotal = 0;
	if (PlayerId != SpecialPlayerIndex::All)
	{
		AShooterPlayerState* PlayerState = GetSortedPlayerState(TeamNum, PlayerId);
		if (PlayerState)
		{
			StatTotal = Getter.Execute(PlayerState);
		}
	} 
	else
	{
		for (RankedPlayerMap::TConstIterator PlayerIt(PlayerStateMaps[TeamNum]); PlayerIt; ++PlayerIt)
		{
			AShooterPlayerState* PlayerState = PlayerIt.Value().Get();
			if (PlayerState)
			{
				StatTotal += Getter.Execute(PlayerState);
			}
		}
	}

	return FText::AsNumber(LerpForCountup(StatTotal)).ToString();
}

int32 SShooterScoreboardWidget::LerpForCountup(int32 ScoreValue) const
{
	if (MatchState > EShooterMatchState::Playing)
	{
		const float LerpAmount = FMath::Min<float>(1.0f, (FPlatformTime::Seconds() - ScoreboardStartTime) / ScoreCountUpTime);
		return FMath::Lerp(0, ScoreValue, LerpAmount);
	}
	else
	{
		return ScoreValue;
	}
}

TSharedRef<SWidget> SShooterScoreboardWidget::MakeTotalsRow(uint8 TeamNum) const
{
	TSharedPtr<SHorizontalBox> TotalsRow;

	SAssignNew(TotalsRow, SHorizontalBox)
	+SHorizontalBox::Slot() .Padding(5)
	[
		SNew(SBorder)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(5)
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTint)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ScoreTotal", "Team Score").ToString())
			.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
		]
	];

	//Leave two columns empty
	for (uint8 i = 0; i < 2; i++)
	{
		TotalsRow->AddSlot() .Padding(5) .AutoWidth() .HAlign(HAlign_Center) .VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(5)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(FStyleDefaults::GetNoBrush())
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(SBox)
				.WidthOverride(ScoreBoxWidth)
				.HAlign(HAlign_Center)
			]
		];
	}
	//Total team score / captures in CTF mode
	TotalsRow->AddSlot() .Padding(5) .AutoWidth() .HAlign(HAlign_Center) .VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Padding(5)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTint)
		[
			SNew(SBox)
			.WidthOverride(ScoreBoxWidth)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SShooterScoreboardWidget::GetStat, Columns.Last().AttributeGetter, TeamNum, SpecialPlayerIndex::All)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
			]
		]
	];

	return TotalsRow.ToSharedRef();
}

TSharedRef<SWidget> SShooterScoreboardWidget::MakePlayerRows(uint8 TeamNum) const
{
	TSharedRef<SVerticalBox> PlayerRows = SNew(SVerticalBox);

	for (int32 PlayerIndex=0; PlayerIndex < PlayerStateMaps[TeamNum].Num(); PlayerIndex++ )
	{
		PlayerRows->AddSlot() .AutoHeight()
		[
			MakePlayerRow(TeamNum, PlayerIndex)
		];
	}

	return PlayerRows;
}

TSharedRef<SWidget> SShooterScoreboardWidget::MakePlayerRow(uint8 TeamNum, int32 PlayerId) const
{
	TSharedPtr<SHorizontalBox> PlayerRow;
	//first autosized row with player name
	SAssignNew(PlayerRow, SHorizontalBox)
	+SHorizontalBox::Slot() .Padding(5)
	[
		SNew(SBorder)
		.Padding(5)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Visibility(this, &SShooterScoreboardWidget::PlayerPresenceToItemVisibility, TeamNum, PlayerId)
		.BorderBackgroundColor(this, &SShooterScoreboardWidget::GetScoreboardBorderColor, TeamNum, PlayerId)
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		[
			SNew(STextBlock)
			.Text(this, &SShooterScoreboardWidget::GetPlayerName, TeamNum, PlayerId)
			.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.StatTextStyle")
		]
	];
	//attributes rows (kills, deaths, score/captures)
	for (uint8 ColIdx = 0; ColIdx < Columns.Num(); ColIdx++)
	{
		PlayerRow->AddSlot()
		.Padding(5) .AutoWidth() .HAlign(HAlign_Center) .VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(5)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Visibility(this, &SShooterScoreboardWidget::PlayerPresenceToItemVisibility, TeamNum, PlayerId)
			.BorderBackgroundColor(this, &SShooterScoreboardWidget::GetScoreboardBorderColor, TeamNum, PlayerId)
			.BorderImage(&ScoreboardStyle->ItemBorderBrush)
			[
				SNew(SBox)
				.WidthOverride(ScoreBoxWidth)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SShooterScoreboardWidget::GetStat, Columns[ColIdx].AttributeGetter, TeamNum, PlayerId)
					.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.StatTextStyle")
					.ColorAndOpacity(Columns[ColIdx].Color)
				]
			]
		];
	}
	return PlayerRow.ToSharedRef();
}

AShooterPlayerState* SShooterScoreboardWidget::GetSortedPlayerState(uint8 TeamNum, int32 SlotIndex) const
{
	if (PlayerStateMaps.IsValidIndex(TeamNum) && PlayerStateMaps[TeamNum].Contains(SlotIndex))
	{
		return PlayerStateMaps[TeamNum].FindRef(SlotIndex).Get();
	}
	
	return NULL;
}

int32 SShooterScoreboardWidget::GetAttributeValue_Kills(AShooterPlayerState* PlayerState) const
{
	return PlayerState->GetKills();
}

int32 SShooterScoreboardWidget::GetAttributeValue_Deaths(AShooterPlayerState* PlayerState) const
{
	return PlayerState->GetDeaths();
}

int32 SShooterScoreboardWidget::GetAttributeValue_Score(AShooterPlayerState* PlayerState) const
{
	return FMath::TruncToInt(PlayerState->Score);
}

#undef LOCTEXT_NAMESPACE
