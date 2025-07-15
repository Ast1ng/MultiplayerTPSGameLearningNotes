#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "¿ÕÏÐ×´Ì¬"),	//¿ÕÏÐ×´Ì¬
	ECS_Reloading UMETA(DisplayName = "»»µ¯ÖÐ"),		//»»µ¯ÖÐ

	ECS_MAX UMETA(DisplayName = "DefaultMax")
};