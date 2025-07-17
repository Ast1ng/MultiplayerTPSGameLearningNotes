// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	//开火
	virtual void Fire(const FVector& HitTarget) override;

protected:
	// 武器扩散
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void  WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere, Category = "武器属性")
	class UParticleSystem* ImpactParticles; //命中粒子效果

	UPROPERTY(EditAnywhere, Category = "武器属性")
	USoundCue* HitSound;					//命中音效

	UPROPERTY(EditAnywhere, Category = "攻击属性")
	float Damage = 45.f;					//伤害值
private:
	UPROPERTY(EditAnywhere, Category = "武器属性")
	UParticleSystem* BeamParticles;			//光束粒子效果

	UPROPERTY(EditAnywhere, Category = "武器属性")
	UParticleSystem* MuzzleFlash;			//枪口火焰粒子效果

	UPROPERTY(EditAnywhere, Category = "武器属性")
	USoundCue* FireSound;					//开火音效

	/*
	*	------- 散射属性 -------
	*/

	UPROPERTY(EditAnywhere, Category = "攻击属性")
	float DistanceToSphere = 800.f;	//到散射距离

	UPROPERTY(EditAnywhere, Category = "攻击属性")
	float SphereRadius = 75.f;	//散射半径

	UPROPERTY(EditAnywhere, Category = "攻击属性")
	bool bUseScatter = false;	//是否使用散射

	UPROPERTY(EditAnywhere, Category = "攻击属性")
	bool bDrawDebugShape = false;	//是否绘制调试用的球体

};
