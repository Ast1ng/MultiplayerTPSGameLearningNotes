// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return; //如果没有拥有者，则返回
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);

			FVector BeamEnd = End;	//光束终点, 即便未命中任何东西也保证有光束粒子生成


			if (FireHit.bBlockingHit)	//造成伤害
			{
				BeamEnd = FireHit.ImpactPoint;	//如果有命中，则将光束终点设置为命中位置
				APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
				if (PlayerCharacter && HasAuthority() && InstigatorController)
				{
					
						UGameplayStatics::ApplyDamage(
						PlayerCharacter,				// 受伤的角色
						Damage,						// 伤害值
						InstigatorController,	// 造成伤害的控制器
						this,						// 造成伤害的武器
						UDamageType::StaticClass()	// 使用默认的伤害类型
					);
					
					
				}
				if (ImpactParticles)	//生成命中粒子效果
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,	//命中位置
						FireHit.ImpactNormal.Rotation(),	//命中法线的旋转
						FVector(1.f),			//缩放
						true					//是否自动销毁
					);
				}
				if (BeamParticles)
				{
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
						World,
						BeamParticles,
						SocketTransform
					);
					if (Beam)
					{
						Beam->SetVectorParameter(FName("Target"), BeamEnd);
					}
				}
			}
		}
	}
	
	

	
		
}
