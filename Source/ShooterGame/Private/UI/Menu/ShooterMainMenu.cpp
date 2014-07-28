// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterMainMenu.h"
#include "ShooterGameLoadingScreen.h"
#include "ShooterStyle.h"
#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterGameKing.h"
#include "Slate.h"
#include "GenericPlatformChunkInstall.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

#define MAX_BOT_COUNT 8

static const FString MapNames[] = { TEXT("Sanctuary"), TEXT("Highrise") };
static const FName PackageNames[] = { TEXT("Sanctuary.umap"), TEXT("Highrise.umap") };
static const int DefaultTDMMap = 1;
static const int DefaultFFAMap = 0; 

//use an EMap index, get back the ChunkIndex that map should be part of.
//Instead of this mapping we should really use the AssetRegistry to query for chunk mappings, but maps aren't members of the AssetRegistry yet.
#if PLATFORM_XBOXONE
static const int ChunkMapping[] = { 1000, 1000 };
#else
static const int ChunkMapping[] = { 1, 2 };
#endif

FShooterMainMenu::~FShooterMainMenu()
{

}

void FShooterMainMenu::Construct(APlayerController* _PCOwner, AShooterGame_Menu* _SGOwner)
{
	bShowingDownloadPct = false;
	PCOwner = _PCOwner;
	SGOwner = _SGOwner;

	// read user settings
	UShooterGameUserSettings* const UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	bIsLanMatch = UserSettings->IsLanMatch();

	UShooterPersistentUser* PersistentUser = GetPersistentUser();	
	BotsCountOpt = 1;
	if(PersistentUser)
	{
		BotsCountOpt = PersistentUser->GetBotsCount();
	}		

	// number entries 0 up to MAX_BOX_COUNT
	TArray<FText> BotsCountList;
	for (int32 i = 0; i <= MAX_BOT_COUNT; i++)
	{
		BotsCountList.Add(FText::AsNumber(i));
	}
	
	TArray<FText> MapList;
	for (int32 i = 0; i < ARRAY_COUNT(MapNames); ++i)
	{
		MapList.Add(FText::FromString(MapNames[i]));		
	}	

	TArray<FText> OnOffList;
	OnOffList.Add( LOCTEXT("Off","OFF") );
	OnOffList.Add( LOCTEXT("On","ON") );

	ShooterOptions = MakeShareable(new FShooterOptions()); 
	ShooterOptions->Construct(PCOwner);
	ShooterOptions->TellInputAboutKeybindings();
	ShooterOptions->OnApplyChanges.BindSP(this, &FShooterMainMenu::CloseSubMenu);

	//Now that we are here, build our menu 
	MenuWidget.Reset();
	MenuWidgetContainer.Reset();

	if (GEngine && GEngine->GameViewport)
	{		
		SAssignNew(MenuWidget, SShooterMenuWidget)
			.Cursor(EMouseCursor::Default)
			.PCOwner(TWeakObjectPtr<APlayerController>(PCOwner))
			.GameModeOwner(SGOwner)
			.IsGameMenu(false);

		
		SAssignNew(MenuWidgetContainer, SWeakWidget)
			.PossiblyNullContent(MenuWidget);		

		TSharedPtr<FShooterMenuItem> RootMenuItem;

				
		SAssignNew(SplitScreenLobbyWidget, SShooterSplitScreenLobby)
			.LocalPlayer(_PCOwner)
			.OnCancelClicked(FOnClicked::CreateSP(this, &FShooterMainMenu::OnSplitScreenBackedOut))
			.OnPlayClicked(FOnClicked::CreateSP(this, &FShooterMainMenu::OnSplitScreenPlay));

		SAssignNew(SplitScreenLobbyWidgetContainer, SWeakWidget)
			.PossiblyNullContent(SplitScreenLobbyWidget);		

#if SHOOTER_CONSOLE_UI
		// PLAY menu option
		TSharedPtr<FShooterMenuItem> PlaySubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Play", "PLAY"));

		// submenu under "play"
		MenuHelper::AddMenuItemSP(PlaySubMenu, LOCTEXT("TDM", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnSplitScreenSelected);
			
		TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(PlaySubMenu, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);
		NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;

		MapOption = MenuHelper::AddMenuOption(PlaySubMenu, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);
		MapOption->SelectedMultiChoice = DefaultTDMMap;

#else
		// HOST menu option
		TSharedPtr<FShooterMenuItem> MenuItem;
		MenuItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Host", "HOST"));

		// submenu under "host"
		MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("FFA", "FREE FOR ALL"), this, &FShooterMainMenu::OnUIHostFreeForAll);
		MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("TDM", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnUIHostTeamDeathMatch);

		TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);				
		NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;																

		MapOption = MenuHelper::AddMenuOption(MenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);

		HostLANItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("LanMatch", "LAN"), OnOffList, this, &FShooterMainMenu::LanMatchChanged);
		HostLANItem->SelectedMultiChoice = bIsLanMatch;

		// JOIN menu option
		MenuItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Join", "JOIN"));

		// submenu under "join"
		MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("Server", "SERVER"), this, &FShooterMainMenu::OnJoinServer);
		JoinLANItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("LanMatch", "LAN"), OnOffList, this, &FShooterMainMenu::LanMatchChanged);
		JoinLANItem->SelectedMultiChoice = bIsLanMatch;

		// Server list widget that will be called up if appropriate
		MenuHelper::AddCustomMenuItem(JoinServerItem,SAssignNew(ServerListWidget,SShooterServerList).OwnerWidget(MenuWidget).PCOwner(PCOwner));
#endif

		// Leaderboards
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Leaderboards", "LEADERBOARDS"), this, &FShooterMainMenu::OnShowLeaderboard);
		MenuHelper::AddCustomMenuItem(LeaderboardItem,SAssignNew(LeaderboardWidget,SShooterLeaderboard).OwnerWidget(MenuWidget).PCOwner(PCOwner));
#endif

		// Options
		MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterOptions->OptionsItem.ToSharedRef());

		if(FSlateApplication::Get().SupportsSystemHelp())
		{
			TSharedPtr<FShooterMenuItem> HelpSubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Help", "HELP"));
			HelpSubMenu->OnConfirmMenuItem.BindStatic([](){ FSlateApplication::Get().ShowSystemHelp(); });
		}

		// QUIT option (for PC)
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FShooterMainMenu::OnUIQuit);
#endif

		MenuWidget->CurrentMenuTitle = LOCTEXT("MainMenu","MAIN MENU");
		MenuWidget->OnGoBack.BindSP(this, &FShooterMainMenu::OnMenuGoBack);
		MenuWidget->MainMenu = MenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		MenuWidget->OnMenuHidden.BindSP(this, &FShooterMainMenu::OnMenuHidden);

		
		ShooterOptions->UpdateOptions();
		MenuWidget->BuildAndShowMenu();

	}
	UpdateMenuOwner();	
}

void FShooterMainMenu::AddMenuToGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		GVC->AddViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
	}
}

void FShooterMainMenu::RemoveMenuFromGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		GVC->RemoveViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
	}
}

void FShooterMainMenu::Tick(float DeltaSeconds)
{
	IPlatformChunkInstall* ChunkInstaller = FPlatformMisc::GetPlatformChunkInstall();
	if (ChunkInstaller)
	{
		EMap SelectedMap = GetSelectedMap();
		// use assetregistry when maps are added to it.
		int32 MapChunk = ChunkMapping[(int)SelectedMap];
		EChunkLocation::Type ChunkLocation = ChunkInstaller->GetChunkLocation(MapChunk);

		FText UpdatedText;
		bool bUpdateText = false;
		if (ChunkLocation == EChunkLocation::NotAvailable)
		{			
			float PercentComplete = FMath::Min(ChunkInstaller->GetChunkProgress(MapChunk, EChunkProgressReportingType::PercentageComplete), 100.0f);									
			UpdatedText = FText::FromString(FString::Printf(TEXT("%s %4.0f%%"),*LOCTEXT("SELECTED_LEVEL", "Map").ToString(), PercentComplete));
			bUpdateText = true;
			bShowingDownloadPct = true;
		}
		else if (bShowingDownloadPct)
		{
			UpdatedText = LOCTEXT("SELECTED_LEVEL", "Map");			
			bUpdateText = true;
			bShowingDownloadPct = false;			
		}

		if (bUpdateText)
		{
			MapOption->SetText(UpdatedText);			
		}
	}
}

bool FShooterMainMenu::IsTickable() const
{
	return true;
}

TStatId FShooterMainMenu::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FShooterMainMenu, STATGROUP_Tickables);
}

bool FShooterMainMenu::IsTickableWhenPaused() const
{
	return true;
}

void FShooterMainMenu::OnMenuHidden()
{	
#if SHOOTER_CONSOLE_UI
	// Menu was hidden from the top-level main menu, on consoles show the welcome screen again.
	UShooterGameKing::Get().SetCurrentState(EShooterGameState::EWelcomeScreen);	
#else
	RemoveMenuFromGameViewport();
#endif
}

void FShooterMainMenu::OnSplitScreenSelected()
{
	if (!IsMapReady())
	{
		return;
	}

	UGameViewportClient* const GVC = GEngine->GameViewport;

	RemoveMenuFromGameViewport();	
	GVC->AddViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());

	SplitScreenLobbyWidget->Clear();
	FSlateApplication::Get().SetKeyboardFocus(SplitScreenLobbyWidget);		
}

FReply FShooterMainMenu::OnSplitScreenBackedOut()
{	
	SplitScreenLobbyWidget->Clear();
	SplitScreenBackedOut();
	return FReply::Handled();
}

FReply FShooterMainMenu::OnSplitScreenPlay()
{
	UGameViewportClient* const GVC = GEngine->GameViewport;
	GVC->RemoveViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());
	OnUIHostTeamDeathMatch();
	return FReply::Handled();
}

void FShooterMainMenu::SplitScreenBackedOut()
{
	UGameViewportClient* const GVC = GEngine->GameViewport;
	GVC->RemoveViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());	
	AddMenuToGameViewport();

	FSlateApplication::Get().SetKeyboardFocus(MenuWidget);	
}

FShooterMainMenu::EMap FShooterMainMenu::GetSelectedMap() const
{
	return (EMap)MapOption->SelectedMultiChoice;
}

void FShooterMainMenu::CloseSubMenu()
{
	MenuWidget->MenuGoBack(true);
}

void FShooterMainMenu::OnMenuGoBack(MenuPtr Menu)
{
	// if we are going back from options menu
	if (ShooterOptions->OptionsItem->SubMenu == Menu)
	{
		ShooterOptions->RevertChanges();
	}
}

void FShooterMainMenu::BotCountOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	BotsCountOpt = MultiOptionIndex;

	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->SetBotsCount(BotsCountOpt);
	}
}

void FShooterMainMenu::LanMatchChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	HostLANItem->SelectedMultiChoice = MultiOptionIndex;
	JoinLANItem->SelectedMultiChoice = MultiOptionIndex;
	bIsLanMatch = MultiOptionIndex > 0;
	UShooterGameUserSettings* UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetLanMatch(bIsLanMatch);
}

void FShooterMainMenu::OnUIHostFreeForAll()
{
	if (!IsMapReady())
	{
		return;
	}

	MenuWidget->LockControls(true);
	MenuWidget->HideMenu();

	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
	MenuHelper::PlaySoundAndCall(PCOwner->GetWorld(),MenuSounds.StartGameSound,OwnerUserIndex,this,&FShooterMainMenu::HostFreeForAll);
}

void FShooterMainMenu::OnUIHostTeamDeathMatch()
{
	if (!IsMapReady())
	{
		return;
	}

	MenuWidget->LockControls(true);
	MenuWidget->HideMenu();

	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
	MenuHelper::PlaySoundAndCall(PCOwner->GetWorld(),MenuSounds.StartGameSound,OwnerUserIndex,this,&FShooterMainMenu::HostTeamDeathMatch);
}


void FShooterMainMenu::HostFreeForAll()
{
	AShooterPlayerController_Menu* const ShooterPC = Cast<AShooterPlayerController_Menu>(PCOwner);
	EMap SelectedMap = GetSelectedMap();
	FString StartStr = FString::Printf(TEXT("/Game/Maps/%s?game=FFA?listen%s?%s=%d"), *MapNames[(int)SelectedMap], bIsLanMatch ? TEXT("?bIsLanMatch") : TEXT(""), *AShooterGameMode::GetBotsCountOptionName(), BotsCountOpt);

	CreateSplitScreenPlayers();

	if (ShooterPC != NULL && ShooterPC->CreateGame(LOCTEXT("FFA","FFA").ToString(), StartStr))
	{		
		FSlateApplication::Get().SetFocusToGameViewport();
		LockAndHideMenu();
		DisplayLoadingScreen();
	}
}

void FShooterMainMenu::HostTeamDeathMatch()
{	
	EMap SelectedMap = GetSelectedMap();
	AShooterPlayerController_Menu * ShooterPC = Cast<AShooterPlayerController_Menu>(PCOwner);
	FString StartStr = FString::Printf(TEXT("/Game/Maps/%s?game=TDM?listen%s?%s=%d"), *MapNames[(int)SelectedMap], bIsLanMatch ? TEXT("?bIsLanMatch") : TEXT(""), *AShooterGameMode::GetBotsCountOptionName(), BotsCountOpt);
	
	CreateSplitScreenPlayers();

	if (ShooterPC != NULL && ShooterPC->CreateGame(LOCTEXT("TDM","TDM").ToString(), StartStr))
	{		
		// Set presence for playing in a map
		if(ShooterPC->PlayerState && ShooterPC->PlayerState->UniqueId.IsValid())
		{
			const auto Presence = Online::GetPresenceInterface();
			if(Presence.IsValid())
			{
				FPresenceProperties Props;
				Props.Add(DefaultPresenceKey, FVariantData(FString("InGame")));
				Presence->SetPresence(*ShooterPC->PlayerState->UniqueId, Props);
			}
		}

		FSlateApplication::Get().SetFocusToGameViewport();
		LockAndHideMenu();
		DisplayLoadingScreen();
	}
}

void FShooterMainMenu::CreateSplitScreenPlayers()
{
	// create a new local players for splitscreen
	// don't use NumReadyPlayers, because there could be a hole in the slot usage.
	for (int i = 1; i < SplitScreenLobbyWidget->GetNumSupportedSlots(); ++i)
	{
		int32 UserIndex = SplitScreenLobbyWidget->GetUserIndexForSlot(i);		
		if (UserIndex != -1)
		{		
			FString Error;
			GEngine->GameViewport->CreatePlayer(UserIndex, Error, false);
		}
	}
}

void FShooterMainMenu::OnJoinServer()
{
	MenuWidget->NextMenu = JoinServerItem->SubMenu;
	ServerListWidget->BeginServerSearch(bIsLanMatch);
	ServerListWidget->UpdateServerList();
	MenuWidget->EnterSubMenu();
}

void FShooterMainMenu::OnShowLeaderboard()
{
	MenuWidget->NextMenu = LeaderboardItem->SubMenu;
	LeaderboardWidget->ReadStats();
	MenuWidget->EnterSubMenu();
}

void FShooterMainMenu::OnUIQuit()
{
	LockAndHideMenu();

	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
	FSlateApplication::Get().PlaySound(MenuSounds.ExitGameSound, OwnerUserIndex);
	MenuHelper::PlaySoundAndCall(PCOwner->GetWorld(),MenuSounds.ExitGameSound,OwnerUserIndex,this,&FShooterMainMenu::Quit);
}

void FShooterMainMenu::Quit()
{
	PCOwner->ConsoleCommand("quit");
}

void FShooterMainMenu::LockAndHideMenu()
{
	MenuWidget->LockControls(true);
	MenuWidget->HideMenu();
}

void FShooterMainMenu::DisplayLoadingScreen()
{
	IShooterGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IShooterGameLoadingScreenModule>("ShooterGameLoadingScreen");
	if( LoadingScreenModule != NULL )
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

void FShooterMainMenu::UpdateMenuOwner()
{
	OwnerUserIndex = 0;
	if (PCOwner != nullptr && PCOwner->IsLocalPlayerController() && PCOwner->Player != nullptr)
	{
		UPlayer * Player = PCOwner->Player;		
		ULocalPlayer * LocalPlayer = CastChecked<ULocalPlayer>(Player);
		OwnerUserIndex = LocalPlayer->ControllerId;		
	}
}

bool FShooterMainMenu::IsMapReady() const
{
	bool bReady = true;
	IPlatformChunkInstall* ChunkInstaller = FPlatformMisc::GetPlatformChunkInstall();
	if (ChunkInstaller)
	{
		EMap SelectedMap = GetSelectedMap();
		// should use the AssetRegistry as soon as maps are added to the AssetRegistry
		int32 MapChunk = ChunkMapping[(int)SelectedMap];
		EChunkLocation::Type ChunkLocation = ChunkInstaller->GetChunkLocation(MapChunk);
		if (ChunkLocation == EChunkLocation::NotAvailable)
		{			
			bReady = false;
		}
	}
	return bReady;
}

UShooterPersistentUser* FShooterMainMenu::GetPersistentUser() const
{
	// Main Menu
	AShooterPlayerController_Menu* ShooterPCM = Cast<AShooterPlayerController_Menu>(PCOwner);
	if(ShooterPCM)
	{
		return ShooterPCM->GetPersistentUser();
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
