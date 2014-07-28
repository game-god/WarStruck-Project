// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameDelegates.h"

#include "ShooterGameKing.h"
#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterMenuWidgetStyle.h"
#include "ShooterMenuItemWidgetStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "ShooterScoreboardWidgetStyle.h"
#include "ShooterChatWidgetStyle.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"



#include "UI/Style/ShooterStyle.h"


class FShooterGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() OVERRIDE
	{
		InitializeShooterGameDelegates();
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		//Hot reload hack
		FSlateStyleRegistry::UnRegisterSlateStyle(FShooterStyle::GetStyleSetName());
		FShooterStyle::Initialize();
		UShooterGameKing::Initialize();
		
	}

	virtual void ShutdownModule() OVERRIDE
	{
		FShooterStyle::Shutdown();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FShooterGameModule, ShooterGame, "ShooterGame");

DEFINE_LOG_CATEGORY(LogShooter)
DEFINE_LOG_CATEGORY(LogShooterWeapon)
