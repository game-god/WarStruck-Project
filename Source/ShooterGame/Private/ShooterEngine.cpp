// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	ShooterEngine.cpp: ShooterEngine c++ code.
=============================================================================*/

#include "ShooterGame.h"

UShooterEngine::UShooterEngine(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}

void UShooterEngine::Init(IEngineLoop* InEngineLoop)
{
	// Note: Lots of important things happen in Super::Init(), including spawning the player pawn in-game and
	// creating the renderer.
	Super::Init(InEngineLoop);	
}


void UShooterEngine::HandleNetworkFailure(UWorld *World, UNetDriver *NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	// Determine if we need to change the King state based on network failures.

	// Only handle failure at this level for game or pending net drivers.
	FName NetDriverName = NetDriver ? NetDriver->NetDriverName : NAME_None;
	if (NetDriverName == NAME_GameNetDriver || NetDriverName == NAME_PendingNetDriver)
	{
		// If this net driver has already been unregistered with this world, then don't handle it.
		if (World)
		{
			UNetDriver * NetDriver = FindNamedNetDriver(World, NetDriverName);
			if (NetDriver)
			{				
				bool bGoBackToMain = false;
				ENetMode FailureNetMode = NetDriver->GetNetMode();	// NetMode of the driver that failed
				switch (FailureType)
				{
					case ENetworkFailure::FailureReceived:
						break;
					case ENetworkFailure::PendingConnectionFailure:						
						break;
					case ENetworkFailure::ConnectionLost:						
					case ENetworkFailure::ConnectionTimeout:
						// only clients need to bail back to main if they lost connection
						bGoBackToMain = (FailureNetMode == NM_Client);									
						break;
					case ENetworkFailure::NetDriverAlreadyExists:
					case ENetworkFailure::NetDriverCreateFailure:
					case ENetworkFailure::OutdatedClient:
					case ENetworkFailure::OutdatedServer:
					default:
						break;
				}

				if (bGoBackToMain)
				{				
					UShooterGameKing& ShooterKing = UShooterGameKing::Get();
					ShooterKing.SetCurrentState(ShooterKing.GetInitialFrontendState());
				}
			}
		}
	}

	// standard failure handling.
	Super::HandleNetworkFailure(World, NetDriver, FailureType, ErrorString);
}

