// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "ShooterGame.h"
#include "ShooterSpectatorPawn.h"

AShooterSpectatorPawn::AShooterSpectatorPawn(const class FPostConstructInitializeProperties& PCIP)
: Super(PCIP)
{	
}

void AShooterSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InputComponent)
{
	check(InputComponent);
	
	InputComponent->BindAxis("MoveForward", this, &ADefaultPawn::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ADefaultPawn::MoveRight);
	InputComponent->BindAxis("MoveUp", this, &ADefaultPawn::MoveUp_World);
	InputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AShooterSpectatorPawn::LookUpAtRate);
}

void AShooterSpectatorPawn::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}
