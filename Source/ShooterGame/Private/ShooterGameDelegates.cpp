// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "GameDelegates.h"


#if !UE_BUILD_SHIPPING

#if PLATFORM_PS4
#include "PS4ChunkInstall.h"
static void PlayGoNext()
{
	IPlatformChunkInstall* ChunkInstaller = FPlatformMisc::GetPlatformChunkInstall();
	if (ChunkInstaller)
	{
		ChunkInstaller->DebugStartNextChunk();
	}
}

FAutoConsoleCommand CmdPlayGoNext(
	TEXT("r.PlayGoNext"),
	TEXT("Tell PlayGo to start downloading the next chunk."),
	FConsoleCommandDelegate::CreateStatic(PlayGoNext)
	);
#endif

#endif


// respond to requests from a companion app
static void WebServerDelegate(int32 UserIndex, const FString& Action, const FString& URL, const TMap<FString, FString>& Params, TMap<FString, FString>& Response)
{
	if (URL == TEXT("/index.html?scoreboard"))
	{
		FString ScoreboardStr = TEXT("{ \"scoreboard\" : [ ");

		// you shouldn't normally use this method to get a UWorld as it won't always be correct in a PIE context.
		// However, the PS4 companion app server will never run in the Editor.
		UGameEngine* GameEngine = CastChecked<UGameEngine>(GEngine);
		if (GameEngine)
		{
			UWorld* World = GameEngine->GetGameWorld();
			if (World)
			{			
				ULocalPlayer *Player = GEngine->GetFirstGamePlayer(World);
				if (Player)
				{
					// get the shoter game
					AShooterGameState* const GameState = Cast<AShooterGameState>(Player->PlayerController->GetWorld()->GameState);


					RankedPlayerMap Players;
					GameState->GetRankedMap(0, Players);

					bool bNeedsComma = false;
					for (auto It = Players.CreateIterator(); It; ++It)
					{
						if (bNeedsComma)
						{
							ScoreboardStr += TEXT(" ,");
						}
						ScoreboardStr += FString::Printf(TEXT(" { \"n\" : \"%s\" , \"k\" : \"%d\" , \"d\" : \"%d\" }"), *It.Value()->GetShortPlayerName(), It.Value()->GetKills(), It.Value()->GetDeaths());
						bNeedsComma = true;
					}
				}

				ScoreboardStr += TEXT(" ] }");

				Response.Add(TEXT("Content-Type"), TEXT("text/html; charset=utf-8"));
				Response.Add(TEXT("Body"), ScoreboardStr);
			}
		}
	}
}

static void AssignStreamingChunk(const FString& PackageToAdd, const FString& LastLoadedMapName, const TArray<int32>& AssetRegistryChunkIDs, const TArray<int32>& ExistingChunkIds, int32& OutChunkIndex)
{
	const int32 BasePak = 0;
	const int32 SanctuaryPak = 1;
	const int32 HighrisePak = 2;

	// Add assets to map paks unless they're engine packages or have already been added to the base (engine) pak.
	if (!PackageToAdd.StartsWith("/Engine/"))
	{
		if ((LastLoadedMapName.Find(TEXT("Sanctuary")) >= 0 || PackageToAdd.Find(TEXT("Sanctuary")) >= 0) &&
				!ExistingChunkIds.Contains(BasePak))
		{
			OutChunkIndex = SanctuaryPak;
		}
		else if ((LastLoadedMapName.Find(TEXT("Highrise")) >= 0 || PackageToAdd.Find(TEXT("Highrise")) >= 0) &&
			!ExistingChunkIds.Contains(BasePak))
		{
			OutChunkIndex = HighrisePak;
		}
	}
	if (OutChunkIndex == INDEX_NONE)
	{
		OutChunkIndex = BasePak;
	}
}

static void ExtendedSaveGameInfoDelegate(const TCHAR* SaveName, const EGameDelegates_SaveGame Key, FString& Value)
{
	static const int32 MAX_SAVEGAME_SIZE = 100 * 1024;
	switch(Key)
	{
		case EGameDelegates_SaveGame::MaxSize:			
			Value = FString::Printf(TEXT("%i"), MAX_SAVEGAME_SIZE);
			break;
		case EGameDelegates_SaveGame::Title:
			Value = TEXT("ShooterGame");
			break;
		case EGameDelegates_SaveGame::SubTitle:
			Value = TEXT("The Shootening");
			break;
		case EGameDelegates_SaveGame::Detail:
			Value = TEXT("ShooterGame User Settings");
			break;
		default:
			break;
	}
}

void InitializeShooterGameDelegates()
{
	FGameDelegates::Get().GetWebServerActionDelegate() = FWebServerActionDelegate::CreateStatic(WebServerDelegate);
	FGameDelegates::Get().GetAssignStreamingChunkDelegate() = FAssignStreamingChunkDelegate::CreateStatic(AssignStreamingChunk);
	FGameDelegates::Get().GetExtendedSaveGameInfoDelegate() = FExtendedSaveGameInfoDelegate::CreateStatic(ExtendedSaveGameInfoDelegate);
}
