// Fill out your copyright notice in the Description page of Project Settings.
// 主要游戏模式
// 处理比赛状态(OnMatchStateSet)、玩家淘汰(PlayerEliminated)、请求重新生成(RequestRespawn)等逻辑



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPSGameMode.generated.h"

namespace MatchState
{
	extern TPSGAME_API const FName Cooldown; // 比赛时间已结束，显示获胜者并开始Cooldown计时器

}

UCLASS()
class TPSGAME_API ATPSGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ATPSGameMode();
	 
	virtual void Tick(float DeltaTime) override;
	//GameMode处理角色淘汰
	virtual void PlayerEliminated(class APlayerCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController); 
	//请求重新生成角色
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	// 热身时间
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; 

	// 比赛时间
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	// 冷却时间
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	// 关卡开始时间
	float LevelStartingTime = 0.f; 

	

protected:
	virtual void BeginPlay() override;

	// 当服务器中比赛状态发生变化时，遍历世界中所有的玩家控制器，并将其比赛状态与服务器同步
	virtual void OnMatchStateSet() override;

private:
	// 倒计时
	float CountdownTime = 0.f;

	

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
