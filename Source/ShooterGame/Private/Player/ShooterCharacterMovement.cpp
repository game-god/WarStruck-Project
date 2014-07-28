// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"


//----------------------------------------------------------------------//
// UPawnMovementComponent
//----------------------------------------------------------------------//
UShooterCharacterMovement::UShooterCharacterMovement(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}


float UShooterCharacterMovement::GetMaxSpeedModifier() const
{
	float SpeedMod = Super::GetMaxSpeedModifier();

	const AShooterCharacter* ShooterCharacterOwner = Cast<AShooterCharacter>(PawnOwner);
	if (ShooterCharacterOwner)
	{
		if (ShooterCharacterOwner->IsTargeting())
		{
			SpeedMod *= ShooterCharacterOwner->GetTargetingSpeedModifier();
		}
		if (ShooterCharacterOwner->IsRunning())
		{
			SpeedMod *= ShooterCharacterOwner->GetRunningSpeedModifier();
		}
	}

	return SpeedMod;
}
