// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterPickup.generated.h"

// Base class for pickup objects that can be placed in the world
UCLASS(abstract)
class AShooterPickup : public AActor
{
	GENERATED_UCLASS_BODY()

	/** pickup on touch */
	virtual void ReceiveActorBeginOverlap(class AActor* Other) OVERRIDE;

	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(class AShooterCharacter* TestPawn) const;

	/** initial setup */
	virtual void BeginPlay() OVERRIDE;

protected:

	/** FX component */
	UPROPERTY(VisibleDefaultsOnly, Category=Effects)
	TSubobjectPtr<UParticleSystemComponent> PickupPSC;

	/** FX of active pickup */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* ActiveFX;

	/** FX of pickup on respawn timer */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* RespawningFX;

	/** sound played when player picks it up */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	USoundCue* PickupSound;

	/** sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	USoundCue* RespawnSound;

	/** how long it takes to respawn? */
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	float RespawnTime;

	/** is it ready for interactions? */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_IsActive)
	uint32 bIsActive:1;

	/* The character who has picked up this pickup */
	UPROPERTY(Transient, Replicated)
	AShooterCharacter* PickedUpBy;

	UFUNCTION()
	void OnRep_IsActive();

	/** give pickup */
	virtual void GivePickupTo(class AShooterCharacter* Pawn);

	/** handle touches */
	void PickupOnTouch(class AShooterCharacter* Pawn);

	/** show and enable pickup */
	virtual void RespawnPickup();

	/** show effects when pickup disappears */
	virtual void OnPickedUp();

	/** show effects when pickup appears */
	virtual void OnRespawned();

	/** blueprint event: pickup disappears */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void OnPickedUpEvent();

	/** blueprint event: pickup appears */
	UFUNCTION(BlueprintImplementableEvent)
	virtual void OnRespawnEvent();
};
