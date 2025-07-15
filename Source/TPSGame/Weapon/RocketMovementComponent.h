// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API URocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
	
protected:

	/*
	* 处理模拟更新期间的阻挡碰撞事件，并验证碰撞后模拟状态的有效性。
	* 若仍在模拟中，则调用 HandleImpact() 函数，并默认返回 EHandleHitWallResult::Deflect，通过 HandleDeflection() 支持多段弹跳与滑动逻辑。
	* 若已停止模拟，则返回 EHandleHitWallResult::Abort，终止后续所有模拟尝试。
	*/
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;


	/*
	* 如果启用弹跳逻辑，则在撞击时影响速度（使用 ComputeBounceResult()）
	* 如果未启用弹跳逻辑或速度低于 BounceVelocityStopSimulatingThreshold 阈值，则停止射弹运动。
	* 触发相关事件（OnProjectileBounce）。
	*/
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

};
