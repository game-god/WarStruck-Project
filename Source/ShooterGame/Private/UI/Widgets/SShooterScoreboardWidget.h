// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Slate.h"

DECLARE_DELEGATE_RetVal_OneParam(int32, FOnGetPlayerStateAttribute, AShooterPlayerState*);

namespace SpecialPlayerIndex
{
	const int32 All = -1;
}

struct FColumnData
{
	/** Column name */
	FString Name;

	/** Column color */
	FSlateColor Color;

	/** Stat value getter delegate */
	FOnGetPlayerStateAttribute AttributeGetter;

	/** defaults */
	FColumnData()
	{
		Color = FLinearColor::White;
	}

	FColumnData(FString InName, FSlateColor InColor, FOnGetPlayerStateAttribute InAtrGetter)
	{
		Name = InName;
		Color = InColor;
		AttributeGetter = InAtrGetter;
	}
};


//class declare
class SShooterScoreboardWidget : public SBorder
{

	SLATE_BEGIN_ARGS(SShooterScoreboardWidget)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, PCOwner)

	SLATE_ATTRIBUTE(EShooterMatchState::Type, MatchState)

	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	/** update PlayerState maps with every tick when scoreboard is shown */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) OVERRIDE;

protected:

	/** updates widgets when players leave or join */
	void UpdateScoreboardGrid();

	/** makes total row widget */
	TSharedRef<SWidget> MakeTotalsRow(uint8 TeamNum) const;

	/** makes player rows */
	TSharedRef<SWidget> MakePlayerRows(uint8 TeamNum) const;

	/** makes player row */
	TSharedRef<SWidget> MakePlayerRow(uint8 TeamNum, int32 PlayerId) const;

	/** updates PlayerState maps to display accurate scores */
	void UpdatePlayerStateMaps();

	/** gets ranked map for specific team */
	void GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const;

	/** gets PlayerState for specific team and player */
	AShooterPlayerState* GetSortedPlayerState(uint8 TeamNum, int32 SlotIndex) const;

	/** get player visibility */
	EVisibility PlayerPresenceToItemVisibility(uint8 TeamNum, int32 PlayerId) const;

	/** get scoreboard border color */
	FSlateColor GetScoreboardBorderColor(uint8 TeamNum, int32 PlayerId) const;

	/** get player name */
	FString GetPlayerName(uint8 TeamNum, int32 PlayerId) const;

	/** get specific stat for team number and optionally player */
	FString GetStat(FOnGetPlayerStateAttribute Getter, uint8 TeamNum, int32 PlayerId=SpecialPlayerIndex::All) const;

	/** linear interpolated score for match outcome animation */
	int32 LerpForCountup(int32 ScoreValue) const;

	/** get match outcome text */
	FString GetMatchOutcomeText() const;

	/** Get text for match-restart notification. */
	FString GetMatchRestartText() const;

	/** get attribute value for kills */
	int32 GetAttributeValue_Kills(class AShooterPlayerState* PlayerState) const;

	/** get attribute value for deaths */
	int32 GetAttributeValue_Deaths(class AShooterPlayerState* PlayerState) const;

	/** get attribute value for score */
	int32 GetAttributeValue_Score(class AShooterPlayerState* PlayerState) const;

	/** scoreboard tint color */
	FLinearColor ScoreboardTint;

	/** width of scoreboard item */
	int32 ScoreBoxWidth;

	/** scoreboard count up time */
	float ScoreCountUpTime;

	/** when the scoreboard was brought up. */
	double ScoreboardStartTime;

	/** the Ranked PlayerState map...cleared every frame */
	TArray<RankedPlayerMap> PlayerStateMaps;

	/** player count in each team in the last tick */
	TArray<int32> LastTeamPlayerCount;

	/** holds player info rows */
	TSharedPtr<SVerticalBox> ScoreboardData;

	/** stat columns data */
	TArray<FColumnData> Columns;

	/** get state of current match */
	EShooterMatchState::Type MatchState;

	/** pointer to our parent HUD */
	TWeakObjectPtr<class APlayerController> PCOwner;

	/** style for the scoreboard */
	const struct FShooterScoreboardStyle *ScoreboardStyle;
};
