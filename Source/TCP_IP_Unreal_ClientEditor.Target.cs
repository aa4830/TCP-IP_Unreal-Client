// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class TCP_IP_Unreal_ClientEditorTarget : TargetRules
{
	public TCP_IP_Unreal_ClientEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;

		ExtraModuleNames.AddRange( new string[] { "TCP_IP_Unreal_Client" } );
	}
}
