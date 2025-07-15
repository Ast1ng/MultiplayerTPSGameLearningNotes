// Fill out your copyright notice in the Description page of Project Settings.
// ��Ҫ��Ϸģʽ
// �������״̬(OnMatchStateSet)�������̭(PlayerEliminated)��������������(RequestRespawn)���߼�



#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPSGameMode.generated.h"

namespace MatchState
{
	extern TPSGAME_API const FName Cooldown; // ����ʱ���ѽ�������ʾ��ʤ�߲���ʼCooldown��ʱ��

}

UCLASS()
class TPSGAME_API ATPSGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	ATPSGameMode();
	 
	virtual void Tick(float DeltaTime) override;
	//GameMode�����ɫ��̭
	virtual void PlayerEliminated(class APlayerCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController); 
	//�����������ɽ�ɫ
	virtual void RequestRespawn(class ACharacter* ElimmedCharacter, AController* ElimmedController);

	// ����ʱ��
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f; 

	// ����ʱ��
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	// ��ȴʱ��
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	// �ؿ���ʼʱ��
	float LevelStartingTime = 0.f; 

	

protected:
	virtual void BeginPlay() override;

	// ���������б���״̬�����仯ʱ���������������е���ҿ����������������״̬�������ͬ��
	virtual void OnMatchStateSet() override;

private:
	// ����ʱ
	float CountdownTime = 0.f;

	

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
