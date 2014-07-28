// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Slate.h"
#include "ShooterHUDPCTrackerBase.h"


/** 
 * A chat widget. Contains a box with history and a text entry box.
 */
class SChatWidget : public SCompoundWidget, public ShooterHUDPCTrackerBase
{
public:
	SLATE_BEGIN_ARGS(SChatWidget)
		: _bKeepVisible(false)
	{}

	/** Should the chat widget be kept visible at all times */
	SLATE_ARGUMENT(bool, bKeepVisible)

	SLATE_END_ARGS()

	/** Needed for every widget */
	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InContext);

	/** Gets the visibility of the entry widget. */
	EVisibility GetEntryVisibility() const;

	/**
	 * Sets the visibility of the chat widgets.
	 *
	 * @param	InVisibility	Required visibility.
	 */
	void SetEntryVisibility( TAttribute<EVisibility> InVisibility );

	/** 
	 * Add a new chat line.
	 *
	 * @param	ChatString		String to add.
	 */
	void AddChatLine(const FString &ChatString);

	TSharedRef<class SWidget> AsWidget();

protected:

	// Struct to hold chat lines.
	struct FChatLine
	{
		// Source string of this chat message.
		FString ChatString;

		FChatLine(const FString& InChatString)
			: ChatString(InChatString)
		{
		}
	};

	/** Update function. Allows us to focus keyboard. */
	void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime);

	/** The UI sets up the appropriate mouse settings upon focus. */
	FReply OnKeyboardFocusReceived( const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent );

	/** 
	 * Delegate called when the text is commited.
	 *
	 * @param	InText			The committed text.
	 * @Param	InCommitInfo	The type of commit (eg. Pressed enter, changed focus)
	 */
	void OnChatTextCommitted(const FText& InText, ETextCommit::Type InCommitInfo);

	/** Return the border color. */
	FSlateColor GetBorderColor() const;

	/** Return the font color. */
	FSlateColor GetChatLineColor() const;

	/** 
	 * Return the adjusted color based on whether the chatbox is visible
	 * 
	 * @param InColor	The color value to use
	 *
	 * @returns The color - with the alpha set to zero if the chatbox is not visible.
	 */
	FSlateColor GetStyleColor( const FLinearColor& InColor ) const;

	TSharedRef<ITableRow> GenerateChatRow(TSharedPtr<FChatLine> ChatLine, const TSharedRef<STableViewBase>& OwnerTable);

	/** Visibility of the entry widget previous frame. */
	EVisibility LastVisibility;

	/** Time to show the chat lines as visible after receiving a chat message. */
	double ChatFadeTime;

	/** When we received our last chat message. */
	double LastChatLineTime;

	/** The edit text widget. */
	TSharedPtr< SEditableTextBox > ChatEditBox;

	/** The chat history list view. */
	TSharedPtr< SListView< TSharedPtr< FChatLine> > > ChatHistoryListView;

	/** The array of chat history. */
	TArray< TSharedPtr< FChatLine> > ChatHistory;

	/** Should this chatbox be kept visible. */
	bool bKeepVisible;

	/** Style to use for this chat widget */
	const struct FShooterChatStyle *ChatStyle;
};



