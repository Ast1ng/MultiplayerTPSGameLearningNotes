// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class TPSGAME_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;	//射弹爆炸事件

protected:
	virtual void BeginPlay() override;
	void StartDestoryTimer();		//开始延迟销毁尾迹计时器
	void DestoryTimerFinished();	//延迟销毁尾迹计时器回调函数
	void SpawnTrailSystem();		//生成射弹尾迹特效
	void ExplodeDamage();			//造成爆炸伤害

	//命中事件（命中目标效果，命中的目标，命中其它物体的效果，其它物体，垂直物体表面的脉冲，命中结果FHitResult)
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
	//武器每发弹药的伤害值
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;	//碰撞粒子

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;	//碰撞音效

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;	//碰撞盒

	UPROPERTY(EditAnywhere, Category = "特效")
	class UNiagaraSystem* TrailSystem;	//射弹尾迹特效

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;	//射弹尾迹组件

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;	//射弹移动组件，仅在子类中实现

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ProjectileMesh;	//射弹模型

	UPROPERTY(EditAnywhere, Category = "伤害属性")
	float InnerRadius = 200.f;	//爆炸内半径

	UPROPERTY(EditAnywhere, Category = "伤害属性")
	float OuterRadius = 500.f;	//爆炸外半径

private:
	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;	//子弹曳光

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;	//曳光组件

	FTimerHandle DestroyTimer;	//延迟销毁尾迹计时器

	UPROPERTY(EditAnywhere, Category = "特效")
	float DestoryTime = 3.f;	//延迟销毁尾迹时间
};
