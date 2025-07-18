﻿#pragma once

//准星方向的射线
#define TRACE_LENGTH 80000.f

#define CUSTOM_DEPTH_PURPLE 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252

UENUM(BlueprintType)
enum class EWeaponType : uint8 //武器类型
{
	EWT_AssaultRifle UMETA(DisplayName = "突击步枪"),	
	EWT_RocketLauncher UMETA(DisplayName = "火箭弹发射器"),
	EWT_Pistol UMETA(DisplayName = "手枪"),
	EWT_SubmachineGun UMETA(DisplayName = "冲锋枪"),
	EWT_Shotgun UMETA(DisplayName = "霰弹枪"),
	EWT_SniperRifle UMETA(DisplayName = "狙击步枪"),
	EWT_GrenadeLauncher UMETA(DisplayName = "榴弹发射器"),

	EWT_MAX UMETA(DisplayName = "DefaultMax")
};