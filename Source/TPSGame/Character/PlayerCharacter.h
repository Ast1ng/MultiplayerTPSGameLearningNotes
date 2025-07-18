// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TPSGame/PlayerTypes/TurningInPlace.h"
#include "TPSGame/Interface/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "TPSGame/PlayerTypes/CombatState.h"
#include "PlayerCharacter.generated.h"


UCLASS()
class TPSGAME_API APlayerCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	// 设置默认角色属性
	APlayerCharacter();
	// 逐帧调用（Tick)
	virtual void Tick(float DeltaTime) override;
	// 调用以将功能绑定到输入
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//注册需要网络复制的属性
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//初始化后的组件
	virtual void PostInitializeComponents() override;
	//播放开火动画蒙太奇
	void PlayFireMontage(bool bAiming);
	//播放换弹蒙太奇
	void PlayReloadMontage();
	//播放阵亡动画蒙太奇
	void PlayElimMontage();

	virtual void OnRep_ReplicatedMovement() override; 

	//仅在服务器调用的角色被淘汰函数
	void Elim();
	//角色被淘汰
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();

	//重载销毁函数
	virtual void Destroyed() override;

	//是否禁用角色控制输入
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

protected:
	// BeginPlay
	virtual void BeginPlay() override;

	//
	// ------- 输入系统 -------
	//
	void MoveForward(float Value);		//前后移动
	void MoveRight(float Value);		//左右移动
	void Turn(float Value);				//转向
	void LookUp(float Value);			//抬头低头
	void EquipButtonPressed();			//按下拾取武器按钮（E）
	void CrouchButtonPressed();			//按下蹲下按钮（Ctrl)
	void ReloadButtonPressed();			//按下换弹按钮（R)
	void AimButtonPressed();			//按下瞄准（鼠标右键）	
	void AimButtonReleased();			//松开瞄准（鼠标右键）
	void AimOffset(float DeltaTime);	//获取瞄准偏航角（用于瞄准偏移动画）
	void CalculateAO_Ptich();

	// ------------------------------

	//模拟代理转身
	void SimProxiesTurn();
	virtual void Jump() override;		//跳跃 从蹲下中站起
	void FireButtonPressed();			//按下开火
	void FireButtonReleased();			//松开开火

	//播放受击动画蒙太奇
	void PlayHitReactMontage();

	//受到伤害事件
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	//更新血量显示HUD
	void UpdateHUDHealth();

	//拉取所有相关类并初始化HUD
	void PollInit();

	void RotateInPlace(float DeltaTime);	//转身
private:
	//弹簧臂
	UPROPERTY(EditAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;	

	//摄像机
	UPROPERTY(EditAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;	

	//显示角色网络类型的UI控件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	//重叠的武器
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;	

	//网络复制重叠的武器
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);	

	//战斗组件
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;	

	//服务端调用的拾取武器按钮触发时的事件
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();	

	//用于瞄准偏移
	float AO_Yaw;		
	float InterpAO_Yaw;
	float AO_Pitch;						
	FRotator StartingAimRotation; //用于将角色的旋转偏移重置到当前准心的方向

	//站立不懂时转向的状态
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);


	/**
	*	------- 动画蒙太奇 -------
	**/
	//开火动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* FireWeaponMontage;	

	//换弹动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ReloadMontage;

	//受击动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	//阵亡动画蒙太奇
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ElimMontage;

	// ------------------------------



	//角色模型离摄像机太近时隐藏
	void HideCameraIfChanracterColse();

	//摄像机与角色模型之间距离的阈值，当距离小于阈值时，角色模型隐藏
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraThreshold = 125.f;

	//是否旋转根骨骼
	bool bRotateRootBone;

	//非本地权限的角色转身时播放转身动画时的阈值
	float TurnThreshold = 1.f;

	//上一帧时的代理旋转角
	FRotator ProxyRotationLastFrame;
	//当前帧的代理旋转角
	FRotator ProxyRotation;
	//代理权限下的偏航角
	float ProxyYaw;
	//从上次网络复制角色的移动组件后经过的时间。若此变量大于一定值，则执行一次网络同步，实现半定期的移动组件同步。
	float TimeSinceLastMovementReplication;
	//计算速度，用于角色旋转动画
	float CalculateSpeed();

	/*
	*	------- 玩家生命值 -------
	*/

	//玩家最大生命值
	UPROPERTY(EditAnywhere, Category = "玩家统计信息")
	float MaxHealth = 100.f;

	//玩家当前生命值
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "玩家统计信息")
	float Health = 100.f;

	//当Health发生变化时触发的网络更新的函数
	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	//角色是否阵亡
	bool bElimmed = false;

	//阵亡定时器句柄
	FTimerHandle ElimTimer;
	//重生时间
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;
	//重生定时器
	void ElimTimerFinished();

	// ------------------------------

	/*
	*	------- 溶解效果 -------
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	//更新溶解材质
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	//开始光边溶解
	void StartDissolve();

	//可在游戏中改变的动态材质
	UPROPERTY(VisibleAnywhere, Category = "淘汰")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_Skin;

	UPROPERTY(VisibleAnywhere, Category = "淘汰")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_Clothes;

	UPROPERTY(VisibleAnywhere, Category = "淘汰")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_Face;

	UPROPERTY(VisibleAnywhere, Category = "淘汰")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_Eyes;

	UPROPERTY(VisibleAnywhere, Category = "淘汰")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance_Hair;


	//蓝图中设置的材质，与动态材质实例结合使用
	UPROPERTY(EditAnywhere, Category = "淘汰")
	UMaterialInstance* DissolveMaterialInstance_Skin;

	UPROPERTY(EditAnywhere, Category = "淘汰")
	UMaterialInstance* DissolveMaterialInstance_Clothes;

	UPROPERTY(EditAnywhere, Category = "淘汰")
	UMaterialInstance* DissolveMaterialInstance_Face;

	UPROPERTY(EditAnywhere, Category = "淘汰")
	UMaterialInstance* DissolveMaterialInstance_Eyes;

	UPROPERTY(EditAnywhere, Category = "淘汰")
	UMaterialInstance* DissolveMaterialInstance_Hair;

	// ------------------------------

	/**
	*	------- 淘汰机器人 -------
	*/

	//淘汰机器人粒子效果
	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	//淘汰机器人粒子系统组件
	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

public:

	//设置需要网络复制的武器（OverlappingWeapon）
	void SetOverlappingWeapon(AWeapon* Weapon);	
	//是否装备武器
	bool IsWeaponEquipped();	
	//是否处于瞄准状态
	bool IsAiming();
	//获取AO_Yaw
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }	
	//获取AO_Pitch
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }	

	AWeapon* GetEquippedWeapon();

	//获取转向状态
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }	

	FVector GetHitTarget() const;

	//获取跟随相机
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }		

	//获取是否允许旋转根骨骼
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	FORCEINLINE bool GetDIsableGameplay() const { return bDisableGameplay; }
};
