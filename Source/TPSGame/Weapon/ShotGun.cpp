// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return; //如果没有拥有者，则返回
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<APlayerCharacter*, int32> HitMap; //记录命中玩家和命中次数
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter && HasAuthority() && InstigatorController)
			{
				//如果命中玩家已经存在于HitMap中，则增加命中次数，否则添加新的玩家和命中次数为1
				if (HitMap.Contains(PlayerCharacter))
				{
					HitMap[PlayerCharacter]++;
				}
				else
				{
					HitMap.Emplace(PlayerCharacter, 1);
				}
			}

			if (ImpactParticles)	//生成命中粒子效果
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					this,
					ImpactParticles,
					FireHit.ImpactPoint,	//命中位置
					FireHit.ImpactNormal.Rotation(),	//命中法线的旋转
					FVector(1.f),			//缩放
					true					//是否自动销毁
				);
			}
			if (HitSound) //只有在没有命中粒子特效来播放命中音效的时候才需要的命中音效
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,	//命中位置
					.5f, //音量
					FMath::FRandRange(-0.5f, 0.5f) //随机音调偏移量
				);
			}
		}
		for (auto HitPair : HitMap)
		{
			if (InstigatorController)
			{
				//对每个命中的玩家应用伤害
				if (HitPair.Key && HasAuthority() && InstigatorController)
				{

					UGameplayStatics::ApplyDamage(
						HitPair.Key,				// 受伤的角色
						Damage * HitPair.Value,		// 伤害值
						InstigatorController,		// 造成伤害的控制器
						this,						// 造成伤害的武器
						UDamageType::StaticClass()	// 使用默认的伤害类型
					);
				}
			}
		}
	
	}
}

