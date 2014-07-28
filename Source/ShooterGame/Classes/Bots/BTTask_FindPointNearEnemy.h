// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BTTask_FindPointNearEnemy.generated.h"

// Bot AI task that tries to find a location near the current enemy
UCLASS()
class UBTTask_FindPointNearEnemy : public UBTTask_BlackboardBase
{
	GENERATED_UCLASS_BODY()

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) OVERRIDE;
};
