// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSGame/HUD/PlayerHUD.h"
#include "TPSGame/Weapon/WeaponTypes.h"
#include "TPSGame/PlayerTypes/CombatState.h"
#include "CombatComponent.generated.h"



class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class APlayerCharacter;	//将玩家类设为友元，允许玩家类能够访问所有成员。
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	//注册需要网络复制的属性
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	//装备武器
	void EquipWeapon(AWeapon* WeaponToEquip);	
	//换弹
	void Reload();

	//用于在蓝图中处理换弹动画结束时的事件
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	//按下开火时
	void FireButtonPressed(bool bPressed);

protected:
	virtual void BeginPlay() override;
	//设置瞄准状态
	void SetAiming(bool bIsAiming);	

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);


	//装备武器时禁用自动 转向 移动方向
	UFUNCTION()
	void OnRep_EquippedWeapon();

	

	void Fire();
	
	//服务器开火
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TracerHitTarget);
	
	//多播开火
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TracerHitTarget);

	//准星追踪
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	//处理换弹过程中的事件（服务端客户端都可用）
	void HandleReload();
	//需要装弹的数量
	int32 AmountToReload();

private:
	//存储客户端的玩家
	UPROPERTY()
	class APlayerCharacter* Character;	
	//存储控制器
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	//存储准心HUD
	UPROPERTY()
	class APlayerHUD* HUD;

	//存储需要被装备的武器
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;	

	//瞄准状态
	UPROPERTY(Replicated)
	bool bAiming;	

	//通常移动速度
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float BaseWalkSpeed;	
	
	//瞄准状态移动速度
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	float AimWalkSpeed;		

	bool bFireButtonPressed;	//开火状态

	/*
	* ———————— HUD 和 Crosshairs准星 ————————
	*/

	//移动准星扩散量
	float CrosshairVelocityFactor;	
	//跳跃准星扩散量
	float CrosshairInAirFactor;	
	//瞄准时的准星扩散量
	float CrosshairAimFactor;	
	//开枪时的额外准星扩散
	float CrosshairShootingFactor;

	//准星瞄准处
	FVector HitTarget;

	//准星
	FHUDPackage HUDPackage;


	/*
	*	———————— 瞄准 和 FOV ————————
	*/
	//默认不开镜情况下的FOV， beginplay时设置到摄像机上的基础FOV
	float DefaultFOV;

	//开镜视野FOV
	UPROPERTY(EditAnywhere, Category = "战斗")
	float ZoomFOV = 30.f;			

	float CurrentFOV;

	//开镜变焦的速度
	UPROPERTY(EditAnywhere, Category = "战斗")
	float ZoomInterpSpeed = 20.f;	

	//开镜变焦
	void InterpFOV(float DeltaTime);	

	/*
	*	———————— 自动开火 ————————
	*/

	//开火计时器句柄
	FTimerHandle FireTimer;
	//能否开火
	bool bCanFire = true;

	//开火时触发开火计时器
	void StartFireTimer();
	//完成开火时的计时器回调函数
	void FireTimerFinished();
	//可以开火（弹夹弹药量不为空，且处于可以开火的状态）
	bool CanFire();

	/*
	*	———————— 武器弹药 ————————
	*/

	//携带的当前装备的武器类型的备弹量
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	TMap<EWeaponType, int32> CarriedAmmoMap;

	//初始突击步枪备弹
	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingARAmmo = 30;

	//初始火箭弹备弹
	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingRocketAmmo = 0;

	//初始手枪备弹
	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingPistolAmmo = 24;

	//初始手枪备弹
	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingSMGAmmo = 30;

	//初始霰弹枪备弹
	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingShotgunAmmo = 10;

	//初始霰弹枪备弹
	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingSniperAmmo = 12;

	UPROPERTY(EditAnywhere, category = "初始备弹")
	int32 StartingGrenadeLauncherAmmo = 10;

	//初始化携带的备弹量
	void InitializeCarrieddAmmo();

	//战斗状态
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoValues();
public:	
	

		
};
