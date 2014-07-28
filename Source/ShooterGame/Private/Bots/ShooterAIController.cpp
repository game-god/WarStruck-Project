// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"

AShooterAIController::AShooterAIController(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
 	BlackboardComp = PCIP.CreateDefaultSubobject<UBlackboardComponent>(this, TEXT("BlackBoardComp"));
 	
 	BehaviorComp = PCIP.CreateDefaultSubobject<UBehaviorTreeComponent>(this, TEXT("BehaviorComp"));

	bWantsPlayerState = true;
}

void AShooterAIController::Possess(APawn* InPawn)
{
	Super::Possess(InPawn);

	AShooterBot* Bot = Cast<AShooterBot>(InPawn);

	// start behavior
	if (Bot && Bot->BotBehavior)
	{
		BlackboardComp->InitializeBlackboard(Bot->BotBehavior->BlackboardAsset);

		EnemyKeyID = BlackboardComp->GetKeyID("Enemy");
		NeedAmmoKeyID = BlackboardComp->GetKeyID("NeedAmmo");

		BehaviorComp->StartTree(Bot->BotBehavior);
	}
}

void AShooterAIController::BeginInactiveState()
{
	Super::BeginInactiveState();

	AGameState* GameState = GetWorld()->GameState;

	const float MinRespawnDelay = (GameState && GameState->GameModeClass) ? GetDefault<AGameMode>(GameState->GameModeClass)->MinRespawnDelay : 1.0f;

	GetWorldTimerManager().SetTimer(this, &AShooterAIController::Respawn, MinRespawnDelay);
}

void AShooterAIController::Respawn()
{
	GetWorld()->GetAuthGameMode()->RestartPlayer(this);
}

void AShooterAIController::FindClosestEnemy()
{
	APawn* MyBot = GetPawn();
	if (MyBot == NULL)
	{
		return;
	}

	const FVector MyLoc = MyBot->GetActorLocation();
	float BestDistSq = MAX_FLT;
	AShooterCharacter* BestPawn = NULL;

	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		AShooterCharacter* TestPawn = Cast<AShooterCharacter>(*It);
		if (TestPawn && TestPawn->IsAlive() && TestPawn->IsEnemyFor(this))
		{
			const float DistSq = (TestPawn->GetActorLocation() - MyLoc).SizeSquared();
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestPawn = TestPawn;
			}
		}
	}

	if (BestPawn)
	{
		SetEnemy(BestPawn);
	}
}

void AShooterAIController::ShootEnemy()
{
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());
	AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}

	bool bCanShoot = false;
	AShooterCharacter* Enemy = GetEnemy();
	if ( Enemy && ( Enemy->IsAlive() )&& (MyWeapon->GetCurrentAmmo() > 0) && ( MyWeapon->CanFire() == true ) )
	{
		if (LineOfSightTo(Enemy, MyBot->GetActorLocation()))
		{
			bCanShoot = true;
		}
	}

	if (bCanShoot)
	{
		MyBot->StartWeaponFire();
	}
	else
	{
		MyBot->StopWeaponFire();
	}
}

void AShooterAIController::CheckAmmo(const class AShooterWeapon* CurrentWeapon)
{
	if (CurrentWeapon && BlackboardComp)
	{
		const int32 Ammo = CurrentWeapon->GetCurrentAmmo();
		const int32 MaxAmmo = CurrentWeapon->GetMaxAmmo();
		const float Ratio = (float) Ammo / (float) MaxAmmo;

		BlackboardComp->SetValueAsBool(NeedAmmoKeyID, (Ratio <= 0.1f));
	}
}

void AShooterAIController::SetEnemy(class APawn* InPawn)
{
	if (BlackboardComp)
	{
		BlackboardComp->SetValueAsObject(EnemyKeyID, InPawn);
		SetFocus(InPawn);
	}
}

class AShooterCharacter* AShooterAIController::GetEnemy() const
{
	if (BlackboardComp)
	{
		return Cast<AShooterCharacter>(BlackboardComp->GetValueAsObject(EnemyKeyID));
	}

	return NULL;
}


void AShooterAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	// Look toward focus
	FVector FocalPoint = GetFocalPoint();
	if( !FocalPoint.IsZero() && GetPawn())
	{
		FVector Direction = FocalPoint - GetPawn()->GetActorLocation();
		FRotator NewControlRotation = Direction.Rotation();
		
		NewControlRotation.Yaw = FRotator::ClampAxis(NewControlRotation.Yaw);

		SetControlRotation(NewControlRotation);

		APawn* const P = GetPawn();
		if (P && bUpdatePawn)
		{
			P->FaceRotation(NewControlRotation, DeltaTime);
		}
		
	}
}

void AShooterAIController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	// Stop the behaviour tree/logic
	BehaviorComp->StopTree();

	// Stop any movement we already have
	StopMovement();

	// Cancel the repsawn timer
	GetWorldTimerManager().ClearTimer(this, &AShooterAIController::Respawn);

	// Clear any enemy
	SetEnemy(NULL);

	// Finally stop firing
	AShooterBot* MyBot = Cast<AShooterBot>(GetPawn());
	AShooterWeapon* MyWeapon = MyBot ? MyBot->GetWeapon() : NULL;
	if (MyWeapon == NULL)
	{
		return;
	}
	MyBot->StopWeaponFire();	
}

