// Copyright Sovreighn Gaming

using UnrealBuildTool;
using System.Collections.Generic;

public class AuraTarget : TargetRules
{
	public AuraTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;

		ExtraModuleNames.AddRange( new string[] { "Aura" } );
	}
}
