// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.


#include "SoundNodeLocalPlayer.generated.h"

/**
 * Choose different branch for sounds attached to locally controlled player
 */
UCLASS(hidecategories=Object, editinlinenew)
class USoundNodeLocalPlayer : public USoundNode
{
	GENERATED_UCLASS_BODY()

	// Begin USoundNode interface.
	virtual void ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances ) OVERRIDE;
	virtual void CreateStartingConnectors( void ) OVERRIDE;
	virtual int32 GetMaxChildNodes() const OVERRIDE;
	virtual int32 GetMinChildNodes() const OVERRIDE;
	virtual FString GetUniqueString() const OVERRIDE;
#if WITH_EDITOR
	virtual FString GetInputPinName(int32 PinIndex) const OVERRIDE;
#endif
	// End USoundNode interface.
};
