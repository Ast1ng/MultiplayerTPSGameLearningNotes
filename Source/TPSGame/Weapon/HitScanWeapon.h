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

private:
	UPROPERTY(EditAnywhere, Category = "武器属性")
	float Damage = 45.f;					//伤害值

	UPROPERTY(EditAnywhere, Category = "武器属性")
	class UParticleSystem* ImpactParticles; //命中粒子效果

	UPROPERTY(EditAnywhere, Category = "武器属性")
	UParticleSystem* BeamParticles;			//光束粒子效果

	UPROPERTY(EditAnywhere, Category = "武器属性")
	UParticleSystem* MuzzleFlash;			//枪口火焰粒子效果

	UPROPERTY(EditAnywhere, Category = "武器属性")
	USoundCue* FireSound;					//开火音效

	UPROPERTY(EditAnywhere, Category = "武器属性")
	USoundCue* HitSound;					//命中音效
};
