// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPSGameState.generated.h"

/**
 * 
 */
UCLASS()
class TPSGAME_API ATPSGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	 virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	 void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);	//更新得分最高的玩家列表

	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;	//得分最高的玩家列表

private:
	float TopScore = 0.f;	//最高得分
};
