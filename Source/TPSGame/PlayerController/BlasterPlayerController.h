// Fill out your copyright notice in the Description page of Project Settings.
// 玩家控制器
// 更新玩家的HUD内容（得分、阵亡数等）
// 同步服务器时间
// 对比赛状态变化进行处理

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	//设置得分
	void SetHUDHealth(float Health, float MaxHealth);
	//设置击杀数
	void SetHUDScore(float Score);
	//设置阵亡数
	void SetHUDDefeats(int32 Defeats);
	//设置弹药数
	void SetHUDWeaponAmmo(int32 Ammo);
	//设置背包弹药量
	void SetHUDCarriedAmmo(int32 Ammo);
	//设置HUD比赛倒计时
	void SetHUDMatchCountdown(float CountdownTime);
	//设置热身状态HUD的倒计时
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//与服务器时间同步，返回理论上的服务器时间(在服务端时为服务端时间，在客户端时为客户端时间加上时间差)
	virtual float GetServerTime();
	//最快速度同步服务器时间
	virtual void ReceivedPlayer() override; 

	// 当比赛状态发生变化时调用
	void OnMatchStateSet(FName State);
	//处理比赛开始时的HUD，隐藏Announcement
	void HandleMatchHasStarted();
	//比赛结束时，显示announcement
	void HandleCooldown();
protected:
	virtual void BeginPlay() override;
	//在HUD中设置比赛剩余时间
	void  SetHUDTime();
	//轮询初始化
	void PollInit();	

	/**
	*	同步服务器和客户端的比赛时间
	**/

	// 请求服务器的正确时间，传入客户端请求的时间
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// 作为对ServerRequestServerTime的响应，返回服务器接收到客户端请求的时间
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// 客户端和服务器时间差
	float ClientServerDelta = 0.f; 


	// 同步服务器时间的频率
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f; 

	// 用于跟踪时间同步的运行时间
	float TimeSyncRunningTime = 0.f; 

	// 检查时间同步
	void CheckTimeSync(float DeltaTime);

	// 服务端RPC检查比赛状态
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// 客户端中途加入比赛时同步比赛状态
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StatingTime);

private:
	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	UPROPERTY()
	class ATPSGameMode* TPSGameMode;

	//比赛持续时间
	float MatchTime = 0.f;

	//热身时间
	float WarmupTime = 0.f;	
	
	//关卡开始时间
	float LevelStartingTime = 0.f;	

	//比赛冷却时间
	float CooldownTime = 0.f;

	//倒计时的整数秒
	uint32 CountdownInt = 0;

	//比赛状态
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;	//角色覆盖层
	bool bInitializeCharacterOverlay = false;	//是否初始化角色覆盖层

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
};
