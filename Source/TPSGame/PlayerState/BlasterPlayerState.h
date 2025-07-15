// Fill out your copyright notice in the Description page of Project Settings.
// 玩家状态
// 更新玩家的HUD内容（得分、阵亡数等）



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
	/**
	*	网络复制变量的通知
	*/
	
	//更新得分
	virtual void OnRep_Score() override;
	//更新阵亡数
	UFUNCTION()
	virtual void OnRep_Defeats();

	//增加分数
	void AddToScore(float ScoreAmount);
	//增加阵亡数
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
	class APlayerCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
 
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	
};
