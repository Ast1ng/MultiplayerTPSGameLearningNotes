// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"




UENUM(BlueprintType)
enum class EWeaponState : uint8	//武器的状态枚举类
{
	EWS_Initial	UMETA(DisplayName = "初始状态"),	//初始状态
	EWS_Equipped UMETA(DisplayName = "已装备"),	//已装备状态
	EWS_Dropped	UMETA(DisplayName = "掉落状态"),	//掉落状态状态

	EWS_MAX UMETA(DisplayName = "MAX")
};

UCLASS()
class TPSGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	//显示“拾取武器”的UI
	void ShowPickupWidget(bool bShowWidget); 
	//开火
	virtual void Fire(const FVector& HitTarget);
	void Dropped();

	//注册需要网络复制的属性
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	//在HUD中显示弹药量
	void SetHUDAmmo();
	//为武器装填子弹
	void AddAmmo(int32 AmmoToAdd);

	/*
		------- 武器 十字准星的纹理 -------
	*/

	UPROPERTY(EditAnywhere, Category = "准星")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "准星")
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "准星")
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "准星")
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "准星")
	UTexture2D* CrosshairsBottom;

	/*
*	开镜时的视野变焦
*/
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/*
	*	------- 全自动开火 -------
	*/

	//开火间隔
	UPROPERTY(EditAnywhere, Category = "武器开火属性")
	float FireDelay = .15f;
	//是否为全自动开火
	UPROPERTY(EditAnywhere, Category = "武器开火属性")
	bool bAutomatic = true;

	// ---------------------------

	/**
	*	------- 武器装备音效 -------
	**/

	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	// ---------------------------


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	

	//重叠时
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);

private:
	UPROPERTY(VisibleAnywhere, Category = "武器属性")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "武器属性")
	class USphereComponent* AreaSphere;
	
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "武器属性")
	EWeaponState WeaponState;
	
	UPROPERTY(VisibleAnywhere, Category = "武器属性")
	class UWidgetComponent* PickupWidget;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "武器属性")
	class UAnimationAsset* FireAnimation;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	//当前子弹数量
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UFUNCTION()
	void OnRep_Ammo();
	
	//减少弹药量并记录
	void SpendRound();

	//弹夹容量
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	UPROPERTY()
	class APlayerCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE	float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	bool IsEmpty();	//弹量是否为空
	bool IsFull();	//弹量是否已满
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
