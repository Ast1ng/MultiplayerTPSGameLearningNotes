// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "TPSGame/Weapon/Weapon.h"
#include "TPSGame/PlayerComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PlayerAnimInstance.h"
#include "TPSGame/TPSGame.h"
#include "TPSGame/PlayerController/BlasterPlayerController.h"
#include "TPSGame/GameMode/TPSGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "TPSGame/PlayerState/BlasterPlayerState.h"
#include "TPSGame/Weapon/WeaponTypes.h"


// 设置默认值   构造函数
APlayerCharacter::APlayerCharacter()
{
 	// 允许逐帧调用（Tick)
	PrimaryActorTick.bCanEverTick = true;

	// 初始化弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));	//创建弹簧臂
	CameraBoom->SetupAttachment(GetMesh());											//附加到网格体上
	CameraBoom->TargetArmLength = 600.f;											//设置弹簧臂长
	CameraBoom->bUsePawnControlRotation = true;										//允许控制器旋转

	// 初始化摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));	//创建摄像机
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		//附加到弹簧臂上
	FollowCamera->bUsePawnControlRotation = false;									//禁用控制器旋转

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));	//创建抬头显示器UI
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));			//创建战斗组件
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;//启用蹲伏
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 900.f);//初始化角色旋转速度为900.f

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;	//初始化旋转状态为不旋转
	NetUpdateFrequency = 66.f;		//初始化网络更新频率为66/s
	MinNetUpdateFrequency = 33.f;	//初始化最小网络更新频率为33/s

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APlayerCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(APlayerCharacter, Health);
	DOREPLIFETIME(APlayerCharacter, bDisableGameplay);
}



void APlayerCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void APlayerCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&APlayerCharacter::ElimTimerFinished,
		ElimDelay
	);
}

void APlayerCharacter::MulticastElim_Implementation()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage(); 

	//设置材质与材质变量属性，并开始光边溶解
	if (DissolveMaterialInstance_Skin)
	{
		DynamicDissolveMaterialInstance_Skin = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Skin, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance_Skin);
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Dissolve"), 0.f);
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Face"), 200.f);
	}

	if (DissolveMaterialInstance_Clothes)
	{
		DynamicDissolveMaterialInstance_Clothes = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Clothes, this);
		GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance_Clothes);
		DynamicDissolveMaterialInstance_Clothes->SetScalarParameterValue(TEXT("Dissolve"), 0.f);
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Face"), 200.f);
	}

	if (DissolveMaterialInstance_Face)
	{
		DynamicDissolveMaterialInstance_Face = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Face, this);
		GetMesh()->SetMaterial(2, DynamicDissolveMaterialInstance_Face);
		DynamicDissolveMaterialInstance_Face->SetScalarParameterValue(TEXT("Dissolve"), 0.f);
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Face"), 200.f);
	}

	if (DissolveMaterialInstance_Eyes)
	{
		DynamicDissolveMaterialInstance_Eyes = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Eyes, this);
		GetMesh()->SetMaterial(3, DynamicDissolveMaterialInstance_Eyes);
		DynamicDissolveMaterialInstance_Eyes->SetScalarParameterValue(TEXT("Dissolve"), 0.f);
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Face"), 200.f);
	}

	if (DissolveMaterialInstance_Hair)
	{
		DynamicDissolveMaterialInstance_Hair = UMaterialInstanceDynamic::Create(DissolveMaterialInstance_Hair, this);
		GetMesh()->SetMaterial(4, DynamicDissolveMaterialInstance_Hair);
		DynamicDissolveMaterialInstance_Hair->SetScalarParameterValue(TEXT("Dissolve"), 0.f);
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Face"), 200.f);
	}

	StartDissolve();

	//禁用角色移动
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately(); 
	if (BlasterPlayerController)
	{
		bDisableGameplay = true; //禁用角色控制输入
		if (Combat)
		{
			Combat->FireButtonPressed(false); //禁用开火
		}
	}

	//关闭碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//生成淘汰机器人
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	bool bHideSniperScore = IsLocallyControlled() && Combat && Combat->EquippedWeapon && Combat->bAiming && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScore)
	{
		ShowSniperScopeWidget(false);
	}
}

void APlayerCharacter::ElimTimerFinished()
{
	ATPSGameMode* TPSGameMode = GetWorld()->GetAuthGameMode<ATPSGameMode>();
	if (TPSGameMode)
	{
		TPSGameMode->RequestRespawn(this, Controller);
	}
	
}

void APlayerCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance_Skin)
	{
		DynamicDissolveMaterialInstance_Skin->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if (DynamicDissolveMaterialInstance_Clothes)
	{
		DynamicDissolveMaterialInstance_Clothes->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if (DynamicDissolveMaterialInstance_Face)
	{
		DynamicDissolveMaterialInstance_Face->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if (DynamicDissolveMaterialInstance_Eyes)
	{
		DynamicDissolveMaterialInstance_Eyes->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}

	if (DynamicDissolveMaterialInstance_Hair)
	{
		DynamicDissolveMaterialInstance_Hair->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void APlayerCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &APlayerCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void APlayerCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	ATPSGameMode* TPSGameMode = Cast<ATPSGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = TPSGameMode && TPSGameMode->GetMatchState() != MatchState::InProgress;
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy(); //如果比赛已结束，则销毁武器
	}
}

// BeginPlay
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &APlayerCharacter::ReceiveDamage);
	}
}


// 逐帧调用（Tick)
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);	//旋转角色
	

	HideCameraIfChanracterColse();
	PollInit();
}

void APlayerCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay) 
	{
		bUseControllerRotationYaw = false;	
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;	//禁用转身
		return;
	}
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Ptich();
	}
}

// 调用以将功能绑定到输入
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//跳跃
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);

	//绑定输入
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);	//前后移动
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);		//左右移动
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);					//左右转向
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);				//上下俯仰
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APlayerCharacter::EquipButtonPressed);	//装备武器
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::CrouchButtonPressed);	//蹲下
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::AimButtonPressed);		//按下瞄准
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::AimButtonReleased);	//松开瞄准
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::FireButtonPressed);		//按下开火
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::FireButtonReleased);	//松开开火
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::ReloadButtonPressed);	//按下换弹
}

//初始化后的组件
void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

//开火动画蒙太奇
void APlayerCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
	
}

//换弹动画蒙太奇
void APlayerCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);

		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GernadeLauncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);

	}
}

void APlayerCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromLeft");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void APlayerCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if (Health == 0.f) 
	{
		ATPSGameMode* TPSGameMode = GetWorld()->GetAuthGameMode<ATPSGameMode>();
		if (TPSGameMode)
		{
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			TPSGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
	
}

//角色前向移动
void APlayerCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);		//获取偏航角
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) );	//根据偏航角创建沿X轴的方向向量
		AddMovementInput(Direction, Value);												//让角色沿X轴移动（前进后退）
	}
}

//角色左右移动
void APlayerCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);		//获取偏航角
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));	//根据偏航角创建沿Y轴的方向向量
		AddMovementInput(Direction, Value);												//让角色沿Y轴移动（左右移动）
	}
}

void APlayerCharacter::Turn(float Value)	//转向
{
	AddControllerYawInput(Value);
}

void APlayerCharacter::LookUp(float Value)	//调整视角
{
	AddControllerPitchInput(Value);
}

void APlayerCharacter::EquipButtonPressed()	//拾取武器
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
		
	}
}

void APlayerCharacter::CrouchButtonPressed()//蹲下站起
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void APlayerCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->Reload();
	}
}

void APlayerCharacter::AimButtonPressed()//按下瞄准
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::AimButtonReleased()//松开瞄准
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

float APlayerCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void APlayerCharacter::AimOffset(float DeltaTime)//瞄准偏移
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	//获取速度
	float Speed = CalculateSpeed();

	//检测是否处于跳跃状态
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	//处于站立状态并且没有跳跃行为
	if (Speed == 0.f && !bIsInAir)	
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation); //偏航角增量
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	
	//移动或跳跃
	if (Speed > 0.f || bIsInAir)	
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);	//重置角色的朝向，使角色朝向准心方向
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Ptich();
}

//计算AO_Pitch
void APlayerCharacter::CalculateAO_Ptich()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//将pitch的值从[270, 360)映射到[-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}


void APlayerCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void APlayerCharacter::Jump()//跳跃
{
	if (bDisableGameplay) return;
	if (bIsCrouched)//蹲下时触发跳跃则站起
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void APlayerCharacter::FireButtonPressed()	//按下开火按钮时设置
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void APlayerCharacter::FireButtonReleased()
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}


//（RPC)服务端触发拾取武器
void APlayerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

//设置转向状态
void APlayerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)//右转状态
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)//左转状态
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)	//当处于转向的状态时，旋转角色的根骨骼从而让角色能够朝向正确的方向
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);	//平滑过渡旋转角
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)	//偏航角小于15°时重置角色的转向状态并设置角色的朝向。
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
	
}


void APlayerCharacter::HideCameraIfChanracterColse()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}



void APlayerCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void APlayerCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void APlayerCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
		}
	}
}





//仅在本地角色上显示"拾取武器"的UI（用于解决服务端不显示的问题）
void APlayerCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{	
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}

	}
}

//网络复制重叠的武器
void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);	//显示“拾取武器”的UI
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//是否装备了武器
bool APlayerCharacter::IsWeaponEquipped()
{

	return (Combat && Combat->EquippedWeapon);
}

//是否处于瞄准状态
bool APlayerCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* APlayerCharacter::GetEquippedWeapon()
{
	if(Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector APlayerCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState APlayerCharacter::GetCombatState() const
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}




