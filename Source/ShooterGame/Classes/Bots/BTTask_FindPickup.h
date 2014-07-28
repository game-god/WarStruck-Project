// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "BTTask_FindPickup.generated.h"

// Bot AI Task that attempts to locate a pickup 
UCLASS()
class UBTTask_FindPickup : public UBTTask_BlackboardBase
{
	GENERATED_UCLASS_BODY()
		
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent* OwnerComp, uint8* NodeMemory) OVERRIDE;
};
