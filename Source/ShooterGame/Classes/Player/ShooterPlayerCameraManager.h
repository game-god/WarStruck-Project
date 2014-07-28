// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterPlayerCameraManager.generated.h"

UCLASS()
class AShooterPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_UCLASS_BODY()

public:

	/** normal FOV */
	float NormalFOV;

	/** targeting FOV */
	float TargetingFOV;

	
};
