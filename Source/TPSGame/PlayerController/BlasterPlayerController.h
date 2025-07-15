// Fill out your copyright notice in the Description page of Project Settings.
// ��ҿ�����
// ������ҵ�HUD���ݣ��÷֡��������ȣ�
// ͬ��������ʱ��
// �Ա���״̬�仯���д���

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
	//���õ÷�
	void SetHUDHealth(float Health, float MaxHealth);
	//���û�ɱ��
	void SetHUDScore(float Score);
	//����������
	void SetHUDDefeats(int32 Defeats);
	//���õ�ҩ��
	void SetHUDWeaponAmmo(int32 Ammo);
	//���ñ�����ҩ��
	void SetHUDCarriedAmmo(int32 Ammo);
	//����HUD��������ʱ
	void SetHUDMatchCountdown(float CountdownTime);
	//��������״̬HUD�ĵ���ʱ
	void SetHUDAnnouncementCountdown(float CountdownTime);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//�������ʱ��ͬ�������������ϵķ�����ʱ��(�ڷ����ʱΪ�����ʱ�䣬�ڿͻ���ʱΪ�ͻ���ʱ�����ʱ���)
	virtual float GetServerTime();
	//����ٶ�ͬ��������ʱ��
	virtual void ReceivedPlayer() override; 

	// ������״̬�����仯ʱ����
	void OnMatchStateSet(FName State);
	//���������ʼʱ��HUD������Announcement
	void HandleMatchHasStarted();
	//��������ʱ����ʾannouncement
	void HandleCooldown();
protected:
	virtual void BeginPlay() override;
	//��HUD�����ñ���ʣ��ʱ��
	void  SetHUDTime();
	//��ѯ��ʼ��
	void PollInit();	

	/**
	*	ͬ���������Ϳͻ��˵ı���ʱ��
	**/

	// �������������ȷʱ�䣬����ͻ��������ʱ��
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// ��Ϊ��ServerRequestServerTime����Ӧ�����ط��������յ��ͻ��������ʱ��
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// �ͻ��˺ͷ�����ʱ���
	float ClientServerDelta = 0.f; 


	// ͬ��������ʱ���Ƶ��
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f; 

	// ���ڸ���ʱ��ͬ��������ʱ��
	float TimeSyncRunningTime = 0.f; 

	// ���ʱ��ͬ��
	void CheckTimeSync(float DeltaTime);

	// �����RPC������״̬
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// �ͻ�����;�������ʱͬ������״̬
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StatingTime);

private:
	UPROPERTY()
	class APlayerHUD* PlayerHUD;

	UPROPERTY()
	class ATPSGameMode* TPSGameMode;

	//��������ʱ��
	float MatchTime = 0.f;

	//����ʱ��
	float WarmupTime = 0.f;	
	
	//�ؿ���ʼʱ��
	float LevelStartingTime = 0.f;	

	//������ȴʱ��
	float CooldownTime = 0.f;

	//����ʱ��������
	uint32 CountdownInt = 0;

	//����״̬
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;	//��ɫ���ǲ�
	bool bInitializeCharacterOverlay = false;	//�Ƿ��ʼ����ɫ���ǲ�

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
};
