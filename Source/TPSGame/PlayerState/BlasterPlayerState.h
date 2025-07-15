// Fill out your copyright notice in the Description page of Project Settings.
// ���״̬
// ������ҵ�HUD���ݣ��÷֡��������ȣ�



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
	*	���縴�Ʊ�����֪ͨ
	*/
	
	//���µ÷�
	virtual void OnRep_Score() override;
	//����������
	UFUNCTION()
	virtual void OnRep_Defeats();

	//���ӷ���
	void AddToScore(float ScoreAmount);
	//����������
	void AddToDefeats(int32 DefeatsAmount);

private:
	UPROPERTY()
	class APlayerCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
 
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	
};
