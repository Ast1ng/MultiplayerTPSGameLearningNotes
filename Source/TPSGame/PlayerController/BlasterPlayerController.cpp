// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "TPSGame/HUD/PlayerHUD.h"
#include "TPSGame/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"
#include "TPSGame/GameMode/TPSGameMode.h"
#include "TPSGame/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "TPSGame/PlayerComponents/CombatComponent.h"
#include "TPSGame/GameState/TPSGameState.h"
#include "TPSGame/PlayerState/BlasterPlayerState.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
	ServerCheckMatchState(); //检查比赛状态
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	
	CheckTimeSync(DeltaTime); //检查时间同步

	PollInit(); //轮询初始化
	
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	//每隔一段时间同步一次服务器时间
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		TimeSyncRunningTime = 0.f;
		//请求服务器时间
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}



void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ATPSGameMode* GameMode = Cast<ATPSGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

		
	}
}

void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StatingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StatingTime;
	CooldownTime = Cooldown;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);

	//如果比赛状态是等待开始，则添加热身状态公告
	if (PlayerHUD && MatchState == MatchState::WaitingToStart)
	{
		PlayerHUD->AddAnnouncement();
	}
}





void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InPawn);
	if (PlayerCharacter)
	{
		SetHUDHealth(PlayerCharacter->GetHealth(), PlayerCharacter->GetMaxHealth());
	}

}



void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	//角色生命值HUD是否可用
	bool bHUDValid = PlayerHUD && 
		PlayerHUD->CharacterOverlay && 
		PlayerHUD->CharacterOverlay->HealthBar && 
		PlayerHUD->CharacterOverlay->HealthText;

	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		PlayerHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		bInitializeCharacterOverlay = true; //如果HUD不可用，则标记需要初始化CharacterOverlay
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		PlayerHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true; //如果HUD不可用，则标记需要初始化CharacterOverlay
		HUDScore = Score;
	}

}


void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		PlayerHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		bInitializeCharacterOverlay = true; //如果HUD不可用，则标记需要初始化CharacterOverlay
		HUDDefeats = Defeats;
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return; //如果倒计时小于0，则不显示倒计时
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{

	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	bool bHUDValid = PlayerHUD &&
		PlayerHUD->Announcement &&
		PlayerHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			PlayerHUD->Announcement->WarmupTime->SetText(FText());
			return; //如果倒计时小于0，则不显示倒计时
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		PlayerHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDTime()
{

	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime; //热身阶段剩余时间
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; //比赛进行中剩余时间
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; //距离下场比赛开始的剩余时间
	}
	//剩余游戏时间
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		TPSGameMode = TPSGameMode == nullptr ? Cast<ATPSGameMode>(UGameplayStatics::GetGameMode(this)) : TPSGameMode;
		if (TPSGameMode)
		{
			SecondsLeft = FMath::CeilToInt(TPSGameMode->GetCountdownTime() + LevelStartingTime); // 获取游戏模式中的倒计时

		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft); //热身状态或冷却状态时，在公告界面中倒计时
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft); //比赛状态时，设置比赛剩余时间倒计时
		}
	}

		CountdownInt = SecondsLeft;

	
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (PlayerHUD && PlayerHUD->CharacterOverlay)
		{
			CharacterOverlay = PlayerHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);	//设置HUD生命值
				SetHUDScore(HUDScore);					//设置HUD分数
				SetHUDDefeats(HUDDefeats);				//设置HUD阵亡数
			}
		}
	}
}



void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	// 服务器接收到客户端请求的时间
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	// 调用客户端函数，将请求时间和服务器接收到的时间发送给客户端
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;	//计算往返时间
	float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);	//计算当前服务器时间
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();	//计算客户端和服务器时间差

}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();					// 返回服务器时间(记录的是玩家控制器被创建的时间点)
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta; // 返回客户端时间加上时间差（服务器上创建其他客户端的玩家控制器的时间点）
	}
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());	// 请求服务器时间
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;

	// 当比赛状态为 InProgress 时，添加玩家覆盖层到 HUD
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(); //服务端处理比赛开始时的 HUD
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}



void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted(); //服务端处理比赛开始时的 HUD
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		if(PlayerHUD->CharacterOverlay == nullptr) PlayerHUD->AddCharacterOverlay();
		if (PlayerHUD->Announcement)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	if (PlayerHUD)
	{
		PlayerHUD->CharacterOverlay->RemoveFromParent(); //移除玩家覆盖层

		bool bHUDValid = PlayerHUD &&
			PlayerHUD->Announcement &&
			PlayerHUD->Announcement->AnnouncementText &&
			PlayerHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			PlayerHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText(TEXT("下场比赛将开始于："));
			PlayerHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ATPSGameState* TPSGameState = Cast<ATPSGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (TPSGameState && BlasterPlayerState)
			{
				TArray<ABlasterPlayerState*> TopPlayers = TPSGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = TEXT("没有赢家！");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
				{
					InfoTextString = TEXT("你是全场最佳！");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("胜利者是： \n%s "), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString::Printf(TEXT("胜利者是：\n"));
					for (auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
				
				PlayerHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
			
		}
	}
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
	if (PlayerCharacter && PlayerCharacter->GetCombat())
	{
		PlayerCharacter->bDisableGameplay = true;
		PlayerCharacter->GetCombat()->FireButtonPressed(false); //禁用开火
	}
}
