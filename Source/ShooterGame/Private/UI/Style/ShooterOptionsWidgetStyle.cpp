// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterOptionsWidgetStyle.h"

FShooterOptionsStyle::FShooterOptionsStyle()
{
}

FShooterOptionsStyle::~FShooterOptionsStyle()
{
}

const FName FShooterOptionsStyle::TypeName(TEXT("FShooterOptionsStyle"));

const FShooterOptionsStyle& FShooterOptionsStyle::GetDefault()
{
	static FShooterOptionsStyle Default;
	return Default;
}

void FShooterOptionsStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
}


UShooterOptionsWidgetStyle::UShooterOptionsWidgetStyle( const class FPostConstructInitializeProperties& PCIP )
	: Super(PCIP)
{
	
}