// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterAIController.generated.h"

UCLASS(config=Game)
class AShooterAIController : public AAIController
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(transient)
	TSubobjectPtr<class UBlackboardComponent> BlackboardComp;

	UPROPERTY(transient)
	TSubobjectPtr<class UBehaviorTreeComponent> BehaviorComp;

	// Begin AController interface
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) OVERRIDE;
	virtual void Possess(class APawn* InPawn) OVERRIDE;
	virtual void BeginInactiveState() OVERRIDE;
	// End APlayerController interface

	void Respawn();

	void CheckAmmo(const class AShooterWeapon* CurrentWeapon);

	void SetEnemy(class APawn* InPawn);

	class AShooterCharacter* GetEnemy() const;

	/* If there is line of sight to current enemy, start firing at them */
	UFUNCTION(BlueprintCallable, Category=Behavior)
	void ShootEnemy();

	/* Finds the closest enemy and sets them as current target */
	UFUNCTION(BlueprintCallable, Category=Behavior)
	void FindClosestEnemy();

	// Begin AAIController interface
	/** Update direction AI is looking based on FocalPoint */
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn = true) OVERRIDE;
	// End AAIController interface

protected:
	int32 EnemyKeyID;
	int32 NeedAmmoKeyID;
};
