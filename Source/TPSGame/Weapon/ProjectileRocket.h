// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();

	virtual void Destroyed() override;	//销毁火箭弹
protected:
	//火箭弹命中事件（命中目标效果，命中的目标，命中其它物体的效果，其它物体，垂直物体表面的脉冲，命中结果FHitResult)
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "特效")
	class UNiagaraSystem* TrailSystem;	//火箭弹尾迹特效

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;	//火箭弹尾迹组件

	void DestoryTimerFinished();	//延迟销毁尾迹计时器回调函数

	UPROPERTY(EditAnywhere, Category = "特效")
	USoundCue* ProjectileLoop; //火箭弹飞行音效

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent; //火箭弹飞行音效组件

	UPROPERTY(EditAnywhere, Category = "特效")
	USoundAttenuation* LoopingSoundAttenuation; //火箭弹飞行音效衰减

	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;	//火箭弹移动组件

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;	//火箭弹模型

	UPROPERTY(EditAnywhere, Category = "伤害属性")
	float InnerRadius = 200.f;	//爆炸内半径

	UPROPERTY(EditAnywhere, Category = "伤害属性")
	float OuterRadius = 500.f;	//爆炸外半径

	FTimerHandle DestroyTimer;	//延迟销毁尾迹计时器

	UPROPERTY(EditAnywhere, Category = "特效")
	float DestoryTime = 3.f;	//延迟销毁尾迹时间
};
