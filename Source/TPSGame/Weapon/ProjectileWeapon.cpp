// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());	// 获取持有武器的Pawn

	// 从武器网格获取枪口闪光插槽(射弹生成的位置)
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	
	if (MuzzleFlashSocket)
	{
		// 获取枪口插槽的Transform（位置/旋转/缩放）
		FTransform SocketTransform =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		
		
		 // 计算从枪口到目标点的向量并生成目标旋转
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();	//设置射弹的拥有者
			SpawnParams.Instigator = InstigatorPawn;	//设置攻击的发起者为持有武器的pawn
			UWorld* World = GetWorld();
			
			if (World)
			{
				// 生成射弹Actor
				World->SpawnActor<AProjectile>(
					ProjectileClass,				// 射弹类
					SocketTransform.GetLocation(),	// 生成位置（枪口）
					TargetRotation,					// 射弹朝向
					SpawnParams						// 生成参数
				);
			}
		}
	}
}
