// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "TPSGame/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
	
protected:
	
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

private:
	UPROPERTY(EditAnywhere, Category = "拾取属性")
	int32 AmmoAmount = 30; // 弹药数量

	UPROPERTY(EditAnywhere, Category = "拾取属性")
	EWeaponType WeaponType;

};
