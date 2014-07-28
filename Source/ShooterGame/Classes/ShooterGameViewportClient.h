// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGameViewportClient.generated.h"

UCLASS(Within=Engine, transient, config=Engine)
class UShooterGameViewportClient : public UGameViewportClient
{
	GENERATED_UCLASS_BODY()

public:

 	// UGameViewportClient interface
 	void NotifyPlayerAdded( int32 PlayerIndex, ULocalPlayer* AddedPlayer ) OVERRIDE;

	virtual void SetReferenceToWorldContext(struct FWorldContext& WorldContext) OVERRIDE;
};