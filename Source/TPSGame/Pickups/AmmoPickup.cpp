// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "TPSGame/PlayerComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UCombatComponent* Combat = PlayerCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupAmmo(WeaponType, AmmoAmount); // 调用 CombatComponent 的 PickupAmmo 方法
		}
	}
	Destroy(); // 销毁弹药
}
