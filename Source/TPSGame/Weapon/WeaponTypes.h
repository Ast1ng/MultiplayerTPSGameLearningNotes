#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8 //武器类型
{
	EWT_AssaultRifle UMETA(DisplayName = "突击步枪"),	
	EWT_RocketLauncher UMETA(DisplayName = "火箭弹发射器"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};