// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return; //如果没有拥有者，则返回
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh()); //获取枪口插槽的Transform（位置/旋转/缩放）
		FVector Start = SocketTransform.GetLocation(); //光束起点为枪口位置

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

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
				FireHit.ImpactPoint	//命中位置
			);
		}
		
		
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform	//枪口火焰位置
			);
		}

		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,					//开火音效
				GetActorLocation()			//音效产生位置
			);
		}
	}	
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f; //如果使用散射，则调用TraceEndWithScatter函数获取终点，否则直接计算终点
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = End;	//射线检测的终点
		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;	//如果有命中，则将光束终点设置为命中位置
		}
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true //是否自动销毁
			);
			if (Beam)
			{
				//创建光束粒子效果并设置目标位置
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector & TraceStart, const FVector & HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();		//到目标的单位向量
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere; //散射位置中心为射线起点到目标点的距离
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); //生成一个随机向量，长度在0到SphereRadius之间
	FVector EndLoc = SphereCenter + RandVec; //散射位置终点为散射位置中心加上随机向量
	FVector ToEndLoc = EndLoc - TraceStart; //从射线起点到散射位置终点的向量

	if (bDrawDebugShape)
	{
		DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, false, 5.f); //在散射位置中心绘制一个调试用的红色球体
		DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, false, 5.f); //在散射位置终点绘制一个调试用的绿色球体
		DrawDebugLine(GetWorld(),
		TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		false,
		5.f);
	}
	

	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH/ ToEndLoc.Size());
}


