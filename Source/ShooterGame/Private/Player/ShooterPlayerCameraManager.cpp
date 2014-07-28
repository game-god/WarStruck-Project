// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"

AShooterPlayerCameraManager::AShooterPlayerCameraManager(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	NormalFOV = 90.0f;
	TargetingFOV = 60.0f;
	ViewPitchMin = -87.0f;
	ViewPitchMax = 87.0f;
	bAlwaysApplyModifiers = true;
}

