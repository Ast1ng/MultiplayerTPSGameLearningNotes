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
	* ����ģ������ڼ���赲��ײ�¼�������֤��ײ��ģ��״̬����Ч�ԡ�
	* ������ģ���У������ HandleImpact() ��������Ĭ�Ϸ��� EHandleHitWallResult::Deflect��ͨ�� HandleDeflection() ֧�ֶ�ε����뻬���߼���
	* ����ֹͣģ�⣬�򷵻� EHandleHitWallResult::Abort����ֹ��������ģ�Ⳣ�ԡ�
	*/
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;


	/*
	* ������õ����߼�������ײ��ʱӰ���ٶȣ�ʹ�� ComputeBounceResult()��
	* ���δ���õ����߼����ٶȵ��� BounceVelocityStopSimulatingThreshold ��ֵ����ֹͣ�䵯�˶���
	* ��������¼���OnProjectileBounce����
	*/
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

};
