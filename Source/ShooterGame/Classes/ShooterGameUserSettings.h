// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGameUserSettings.generated.h"

UCLASS()
class UShooterGameUserSettings : public UGameUserSettings
{
	GENERATED_UCLASS_BODY()

	/** Applies all current user settings to the game and saves to permanent storage (e.g. file). */
	virtual void ApplySettings() OVERRIDE;

	int32 GetGraphicsQuality() const
	{
		return GraphicsQuality;
	}

	void SetGraphicsQuality(int32 InGraphicsQuality)
	{
		GraphicsQuality = InGraphicsQuality;
	}

	bool IsLanMatch() const
	{
		return bIsLanMatch;
	}

	void SetLanMatch(bool InbIsLanMatch)
	{
		bIsLanMatch = InbIsLanMatch;
	}

	/** Gets current fullscreen mode */
	EWindowMode::Type GetCurrentFullscreenMode() const;

	// interface UGameUserSettings
	virtual void SetToDefaults() OVERRIDE;

private:
	/**
	 * Graphics Quality
	 *	0 = Low
	 *	1 = High
	 */
	UPROPERTY(config)
	int32 GraphicsQuality;

	/** is lan match? */
	UPROPERTY(config)
	bool bIsLanMatch;
};