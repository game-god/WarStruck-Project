// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterServerList.h"
#include "SHeaderRow.h"
#include "ShooterStyle.h"
#include "ShooterGameLoadingScreen.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void SShooterServerList::Construct(const FArguments& InArgs)
{
	PCOwner = InArgs._PCOwner;
	OwnerWidget = InArgs._OwnerWidget;
	bSearchingForServers = false;
	bLANMatchSearch = false;
	StatusText = FString();
	BoxWidth = 125;


	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)  
			.WidthOverride(600)
			.HeightOverride(300)
			[
				SAssignNew(ServerListWidget, SListView<TSharedPtr<FServerEntry>>)
				.ItemHeight(20)
				.ListItemsSource(&ServerList)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SShooterServerList::MakeListViewWidget)
				.OnSelectionChanged(this, &SShooterServerList::EntrySelectionChanged)
				.OnMouseButtonDoubleClick(this,&SShooterServerList::OnListItemDoubleClicked)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("ServerName").FixedWidth(BoxWidth*2) .DefaultLabel(NSLOCTEXT("ServerList", "ServerNameColumn", "Server Name"))
					+ SHeaderRow::Column("GameType") .DefaultLabel(NSLOCTEXT("ServerList", "GameTypeColumn", "Game Type"))
					+ SHeaderRow::Column("Players") .DefaultLabel(NSLOCTEXT("ServerList", "PlayersColumn", "Players"))
					+ SHeaderRow::Column("Ping") .DefaultLabel(NSLOCTEXT("ServerList", "NetworkPingColumn", "Ping")))
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SShooterServerList::GetBottomText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuServerListTextStyle")
			]
		]
		
	];
}

/** 
 * Get the current game
 */
AShooterGame_Menu* SShooterServerList::GetGame() const
{
	const APlayerController* const Owner = PCOwner.Get();
	if (Owner != NULL)
	{
		UWorld* const World = Owner->GetWorld();
		if (World)
		{
			return World->GetAuthGameMode<AShooterGame_Menu>();
		}
	}

	return NULL;
}


/** 
 * Get the current game session
 */
AShooterGameSession* SShooterServerList::GetGameSession() const
{
	AShooterGame_Menu * ShooterGI = GetGame();
	if (ShooterGI)
	{
		return Cast<AShooterGameSession>(ShooterGI->GameSession);
	}

	return NULL;
}

/** Updates current search status */
void SShooterServerList::UpdateSearchStatus()
{
	check(bSearchingForServers); // should not be called otherwise

	bool bFinishSearch = true;
	AShooterGameSession* ShooterSession = GetGameSession();
	if (ShooterSession)
	{
		int32 CurrentSearchIdx, NumSearchResults;
		EOnlineAsyncTaskState::Type SearchState = ShooterSession->GetSearchResultStatus(CurrentSearchIdx, NumSearchResults);

		UE_LOG(LogOnlineGame, Log, TEXT("ShooterSession->GetSearchResultStatus: %s"), EOnlineAsyncTaskState::ToString(SearchState) );

		switch(SearchState)
		{
			case EOnlineAsyncTaskState::InProgress:
				StatusText = LOCTEXT("Searching","SEARCHING...").ToString();
				bFinishSearch = false;
				break;

			case EOnlineAsyncTaskState::Done:
				// copy the results
				{
					ServerList.Empty();
					const TArray<FOnlineSessionSearchResult> & SearchResults = ShooterSession->GetSearchResults();
					check(SearchResults.Num() == NumSearchResults);
					if (NumSearchResults == 0)
					{
						StatusText = LOCTEXT("NoServersFound","NO SERVERS FOUND, PRESS SPACE TO TRY AGAIN").ToString();
					} else
					{
						StatusText = LOCTEXT("ServersRefresh","PRESS SPACE TO REFRESH SERVER LIST").ToString();
					}

					for (int32 IdxResult = 0; IdxResult < NumSearchResults; ++IdxResult)
					{
						TSharedPtr<FServerEntry> NewServerEntry = MakeShareable(new FServerEntry());

						const FOnlineSessionSearchResult & Result = SearchResults[IdxResult];

						NewServerEntry->ServerName = Result.Session.OwningUserName;
						NewServerEntry->Ping = FString::FromInt(Result.PingInMs);
						NewServerEntry->CurrentPlayers = FString::FromInt(Result.Session.SessionSettings.NumPublicConnections 
							+ Result.Session.SessionSettings.NumPrivateConnections 
							- Result.Session.NumOpenPublicConnections 
							- Result.Session.NumOpenPrivateConnections);
						NewServerEntry->MaxPlayers = FString::FromInt(ShooterSession->DEFAULT_NUM_PLAYERS);
						NewServerEntry->SearchResultsIndex = IdxResult;
					
						Result.Session.SessionSettings.Get(SETTING_GAMEMODE, NewServerEntry->GameType);

						ServerList.Add(NewServerEntry);
					}
				}
				break;

			case EOnlineAsyncTaskState::Failed:
				// intended fall-through
			case EOnlineAsyncTaskState::NotStarted:
				// intended fall-through
			default:
				break;
		}
	}

	if (bFinishSearch)
	{		
		OnServerSearchFinished();
	}
}


FString SShooterServerList::GetBottomText() const
{
	 return StatusText;
}

/**
 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
 *
 * @param  AllottedGeometry The space allotted for this widget
 * @param  InCurrentTime  Current absolute real time
 * @param  InDeltaTime  Real time passed since last tick
 */
void SShooterServerList::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if ( bSearchingForServers )
	{
		UpdateSearchStatus();
	}
}

/** Starts searching for servers */
void SShooterServerList::BeginServerSearch(bool bLANMatch)
{
	bLANMatchSearch = bLANMatch;
	bSearchingForServers = true;
	ServerList.Empty();

	AShooterGame_Menu * Game = GetGame();
	if (Game)
	{
		Game->FindSessions(PCOwner.Get(), bLANMatchSearch);
	}
}

/** Called when server search is finished */
void SShooterServerList::OnServerSearchFinished()
{
	bSearchingForServers = false;

	UpdateServerList();
}

void SShooterServerList::UpdateServerList()
{
	int32 SelectedItemIndex = ServerList.IndexOfByKey(SelectedItem);

	ServerListWidget->RequestListRefresh();
	if (ServerList.Num() > 0)
	{
		ServerListWidget->UpdateSelectionSet();
		ServerListWidget->SetSelection(ServerList[SelectedItemIndex > -1 ? SelectedItemIndex : 0],ESelectInfo::OnNavigation);
	}

}

void SShooterServerList::ConnectToServer()
{
	if (bSearchingForServers)
	{
		// unsafe
		return;
	}

	if (SelectedItem.IsValid())
	{
		int ServerToJoin = SelectedItem->SearchResultsIndex;

		AShooterGame_Menu * Game = GetGame();
		if (Game)
		{
			if (GEngine && GEngine->GameViewport)
			{
				GEngine->GameViewport->RemoveAllViewportWidgets();
			}
			// Ditch the menu and bring loading screen up
			LockControls(true);
			HideMenu();
			IShooterGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IShooterGameLoadingScreenModule>("ShooterGameLoadingScreen");
			if( LoadingScreenModule != NULL )
			{
				LoadingScreenModule->StartInGameLoadingScreen();
			}

			Game->JoinSession(PCOwner.Get(), ServerToJoin);
		}
	}
}

void SShooterServerList::OnKeyboardFocusLost( const FKeyboardFocusEvent& InKeyboardFocusEvent )
{
	if (InKeyboardFocusEvent.GetCause() != EKeyboardFocusCause::SetDirectly)
	{
		FSlateApplication::Get().SetKeyboardFocus(SharedThis( this ));
	}
}

FReply SShooterServerList::OnKeyboardFocusReceived(const FGeometry& MyGeometry, const FKeyboardFocusEvent& InKeyboardFocusEvent)
{
	return FReply::Handled().SetKeyboardFocus(ServerListWidget.ToSharedRef(),EKeyboardFocusCause::SetDirectly).CaptureJoystick(SharedThis( this ));
}

void SShooterServerList::EntrySelectionChanged(TSharedPtr<FServerEntry> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedItem = InItem;
}

void SShooterServerList::OnListItemDoubleClicked(TSharedPtr<FServerEntry> InItem)
{
	SelectedItem = InItem;
	ConnectToServer();
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

void SShooterServerList::MoveSelection(int32 MoveBy)
{
	int32 SelectedItemIndex = ServerList.IndexOfByKey(SelectedItem);

	if (SelectedItemIndex+MoveBy > -1 && SelectedItemIndex+MoveBy < ServerList.Num())
	{
		ServerListWidget->SetSelection(ServerList[SelectedItemIndex+MoveBy]);
	}
}

FReply SShooterServerList::OnControllerButtonPressed(const FGeometry& MyGeometry, const FControllerEvent& ControllerEvent)
{
	if (bSearchingForServers) // lock input
	{
		return FReply::Handled();
	}

	FReply Result = FReply::Unhandled();
	const FKey Key = ControllerEvent.GetEffectingButton();

	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		ConnectToServer();
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
	return Result.IsEventHandled() ? Result : OwnerWidget->OnControllerButtonPressed(MyGeometry,ControllerEvent);
}

FReply SShooterServerList::OnKeyDown(const FGeometry& MyGeometry, const FKeyboardEvent& InKeyboardEvent) 
{
	if (bSearchingForServers) // lock input
	{
		return FReply::Handled();
	}

	FReply Result = FReply::Unhandled();
	const FKey Key = InKeyboardEvent.GetKey();
	if (Key == EKeys::Enter)
	{
		ConnectToServer();
		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
	//hit space bar to search for servers again / refresh the list, only when not searching already
	else if (Key == EKeys::SpaceBar)
	{
		BeginServerSearch(bLANMatchSearch);
	}
	return Result;
}

TSharedRef<ITableRow> SShooterServerList::MakeListViewWidget(TSharedPtr<FServerEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SServerEntryWidget : public SMultiColumnTableRow< TSharedPtr<FServerEntry> >
	{
	public:
		SLATE_BEGIN_ARGS(SServerEntryWidget){}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FServerEntry> InItem)
		{
			Item = InItem;
			SMultiColumnTableRow< TSharedPtr<FServerEntry> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			FString ItemText;
			if (ColumnName == "ServerName")
			{
				ItemText = Item->ServerName;
			}
			else if (ColumnName == "GameType")
			{
				ItemText = Item->GameType;
			}
			else if (ColumnName == "Players")
			{
				const FText PlayersOnServer = FText::Format( FText::FromString("{0}/{1}"), FText::FromString(Item->CurrentPlayers), FText::FromString(Item->MaxPlayers) );
				ItemText = PlayersOnServer.ToString();
			}
			else if (ColumnName == "Ping")
			{
				ItemText = Item->Ping;
			} 
			return SNew(STextBlock)
				.Text(ItemText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuServerListTextStyle");
		}
		TSharedPtr<FServerEntry> Item;
	};
	return SNew(SServerEntryWidget, OwnerTable, Item);
}

#undef LOCTEXT_NAMESPACE