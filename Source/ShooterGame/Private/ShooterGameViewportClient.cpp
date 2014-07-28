// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"

UShooterGameViewportClient::UShooterGameViewportClient(const FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
	SetSuppressTransitionMessage(true);
}

void UShooterGameViewportClient::NotifyPlayerAdded(int32 PlayerIndex, ULocalPlayer* AddedPlayer)
{
	Super::NotifyPlayerAdded(PlayerIndex, AddedPlayer);

 	UShooterLocalPlayer* const ShooterLP = Cast<UShooterLocalPlayer>(AddedPlayer);
 	if (ShooterLP)
 	{
 		ShooterLP->LoadPersistentUser();
 	}
}

void UShooterGameViewportClient::SetReferenceToWorldContext(struct FWorldContext& WorldContext)
{
	Super::SetReferenceToWorldContext(WorldContext);
	if (World && World->IsPlayInEditor())
	{
		// in PIE, there's not necessarily anything to decide the appropriate state other than which map you're loading.
		// You may not have a GameMode as a client, and you may be bypassing the ShooterEntry map.
		FString MapName = WorldContext.LastURL.Map;
		UShooterGameKing& ShooterKing = UShooterGameKing::Get();
		if (MapName.Contains(TEXT("ShooterEntry")))
		{			
			ShooterKing.SetCurrentState(ShooterKing.GetInitialFrontendState());
		}
		else
		{
			ShooterKing.SetCurrentState(EShooterGameState::EPlaying);
		}
	}
}
