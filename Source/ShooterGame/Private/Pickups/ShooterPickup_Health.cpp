// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"

AShooterPickup_Health::AShooterPickup_Health(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	Health = 50;
}

bool AShooterPickup_Health::CanBePickedUp(class AShooterCharacter* TestPawn) const
{
	return TestPawn && (TestPawn->Health < TestPawn->GetMaxHealth());
}

void AShooterPickup_Health::GivePickupTo(class AShooterCharacter* Pawn)
{
	if (Pawn)
	{
		Pawn->Health = FMath::Min(FMath::TruncToInt(Pawn->Health) + Health, Pawn->GetMaxHealth());
	}
}
