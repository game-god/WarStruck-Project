// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "../UI/Style/ShooterStyle.h"


AShooterPlayerController_Menu::AShooterPlayerController_Menu(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
}

void AShooterPlayerController_Menu::PostInitializeComponents() 
{
	Super::PostInitializeComponents();

	FShooterStyle::Initialize();
}


/** 
 * Creates online game owned by this controller
 * @param Type game type (specific to ShooterGame)
 */
bool AShooterPlayerController_Menu::CreateGame(const FString & Type, const FString & InTravelURL)
{
	UWorld* const World = GetWorld();
	AShooterGame_Menu* const GameMode = World ? World->GetAuthGameMode<AShooterGame_Menu>() : NULL;

	return GameMode ? GameMode->HostGame(this, Type, InTravelURL) : false;
}


UShooterPersistentUser* AShooterPlayerController_Menu::GetPersistentUser() const
{
	UShooterLocalPlayer* const ShooterLP = Cast<UShooterLocalPlayer>(Player);
	if (ShooterLP)
	{
		// Initial player gets created before the world, which can cause the persistentuser to not be present.
		// Ensure we have a good PersistenUser.
		if (!ShooterLP->PersistentUser)
		{
			ShooterLP->LoadPersistentUser();
		}
		return ShooterLP->PersistentUser;
	}

	return NULL;
}
