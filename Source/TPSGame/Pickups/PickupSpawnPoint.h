// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class TPSGAME_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses; //可生成的道具列表

	UPROPERTY()
	APickup* SpawnedPickup; // 要生成的道具

	void SpawnPickup(); // 生成道具
	void SpawnPickupTimerFinished(); // 生成道具定时器回调函数

	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor); // 启动生成道具定时器
private:
	FTimerHandle SpawnPickupTimer; // 生成道具定时器句柄

	UPROPERTY(EditAnywhere, Category = "生成设置")
	float SpawnPickupTimeMin; // 最小生成时间

	UPROPERTY(EditAnywhere, Category = "生成设置")
	float SpawnPickupTimeMax; // 最大生成时间

};
