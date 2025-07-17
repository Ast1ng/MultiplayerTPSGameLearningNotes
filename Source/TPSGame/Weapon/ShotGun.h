// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
	
public:
	//开火
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere, Category = "攻击属性")
	uint32 NumberOfPellets = 10; //每次开火发射的子弹数量
};
