// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterTypes.h"
#include "ShooterPlayerController_Menu.generated.h"

UCLASS()
class AShooterPlayerController_Menu : public APlayerController
{
	GENERATED_UCLASS_BODY()

	/** After game is initialized */
	virtual void PostInitializeComponents() OVERRIDE;

	/** 
	 * Creates online game owned by this controller 
	 *
	 * @param Type game type (specific to ShooterGame)
	 */
	bool CreateGame(const FString & Type, const FString & InTravelURL);


	UShooterPersistentUser* GetPersistentUser() const;
};

