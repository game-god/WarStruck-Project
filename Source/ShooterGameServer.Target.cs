// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ShooterGameServerTarget : TargetRules
{
	public ShooterGameServerTarget(TargetInfo Target)
	{
		Type = TargetType.Server;
		bUsesSteam = true;
	}

	//
	// TargetRules interface.
	//

	public override bool GetSupportedPlatforms(ref List<UnrealTargetPlatform> OutPlatforms)
	{
		// It is valid for only server platforms
		return UnrealBuildTool.UnrealBuildTool.GetAllServerPlatforms(ref OutPlatforms, false);
	}

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.Add("ShooterGame");
	}
    public override List<UnrealTargetPlatform> GUBP_GetPlatforms_MonolithicOnly(UnrealTargetPlatform HostPlatform)
    {
        if (HostPlatform == UnrealTargetPlatform.Mac)
        {
            return new List<UnrealTargetPlatform>();
        }
        return new List<UnrealTargetPlatform> { HostPlatform, UnrealTargetPlatform.Linux };
    }

    public override List<UnrealTargetConfiguration> GUBP_GetConfigs_MonolithicOnly(UnrealTargetPlatform HostPlatform, UnrealTargetPlatform Platform)
    {
        return new List<UnrealTargetConfiguration> { UnrealTargetConfiguration.Development, UnrealTargetConfiguration.Shipping };
    }
#if false
    public override List<UnrealTargetConfiguration> GUBP_GetConfigsForFormalBuilds_MonolithicOnly(UnrealTargetPlatform HostPlatform, UnrealTargetPlatform Platform)
    {
        if (HostPlatform == UnrealTargetPlatform.Win64)
        {
            if (Platform == UnrealTargetPlatform.Win64 || Platform == UnrealTargetPlatform.Linux)
            {
                return new List<UnrealTargetConfiguration> { UnrealTargetConfiguration.Development };
            }
        }
        else
        {
        }
        return new List<UnrealTargetConfiguration>();
    }
#endif
}
