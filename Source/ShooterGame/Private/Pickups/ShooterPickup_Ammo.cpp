// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"

AShooterPickup_Ammo::AShooterPickup_Ammo(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	AmmoClips = 2;
}

bool AShooterPickup_Ammo::IsForWeapon(UClass* WeaponClass)
{
	return WeaponType->IsChildOf(WeaponClass);
}

bool AShooterPickup_Ammo::CanBePickedUp(class AShooterCharacter* TestPawn) const
{
	AShooterWeapon* TestWeapon = (TestPawn ? TestPawn->FindWeapon(WeaponType) : NULL);
	if (bIsActive && TestWeapon)
	{
		return TestWeapon->GetCurrentAmmo() < TestWeapon->GetMaxAmmo();
	}

	return false;
}

void AShooterPickup_Ammo::GivePickupTo(class AShooterCharacter* Pawn)
{
	AShooterWeapon* Weapon = (Pawn ? Pawn->FindWeapon(WeaponType) : NULL);
	if (Weapon)
	{
		Weapon->GiveAmmo(AmmoClips * Weapon->GetAmmoPerClip());
	}
}
