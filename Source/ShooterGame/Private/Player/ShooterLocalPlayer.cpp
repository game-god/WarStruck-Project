// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "OnlineSubsystemUtilsClasses.h"

UShooterLocalPlayer::UShooterLocalPlayer(const class FPostConstructInitializeProperties& PCIP)
	: Super(PCIP)
{
}

TSubclassOf<UOnlineSession> UShooterLocalPlayer::GetOnlineSessionClass()
{
	return UOnlineSessionClient::StaticClass();
}

void UShooterLocalPlayer::LoadPersistentUser()
{
	if (PersistentUser == NULL)
	{
		PersistentUser = UShooterPersistentUser::LoadPersistentUser( GetNickname(), ControllerId );
	}
}

void UShooterLocalPlayer::SetControllerId(int32 NewControllerId)
{
	ULocalPlayer::SetControllerId(NewControllerId);

	// if we changed controllerid / user, then we need to load the appropriate persistent user.
	if (PersistentUser != nullptr && ( ControllerId != PersistentUser->GetUserIndex() || GetNickname() != PersistentUser->GetName() ) )
	{
		PersistentUser->SaveIfDirty();
		PersistentUser = nullptr;
	}

	if (!PersistentUser)
	{
		LoadPersistentUser();
	}
}

