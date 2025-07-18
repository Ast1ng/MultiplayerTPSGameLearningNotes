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


// ����Ĭ��ֵ   ���캯��
APlayerCharacter::APlayerCharacter()
{
 	// ������֡���ã�Tick)
	PrimaryActorTick.bCanEverTick = true;

	// ��ʼ�����ɱ�
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));	//�������ɱ�
	CameraBoom->SetupAttachment(GetMesh());											//���ӵ���������
	CameraBoom->TargetArmLength = 600.f;											//���õ��ɱ۳�
	CameraBoom->bUsePawnControlRotation = true;										//�����������ת

	// ��ʼ�������
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));	//���������
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);		//���ӵ����ɱ���
	FollowCamera->bUsePawnControlRotation = false;									//���ÿ�������ת

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));	//����̧ͷ��ʾ��UI
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));			//����ս�����
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;//���ö׷�
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 900.f);//��ʼ����ɫ��ת�ٶ�Ϊ900.f

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;	//��ʼ����ת״̬Ϊ����ת
	NetUpdateFrequency = 66.f;		//��ʼ���������Ƶ��Ϊ66/s
	MinNetUpdateFrequency = 33.f;	//��ʼ����С�������Ƶ��Ϊ33/s

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

	//���ò�������ʱ������ԣ�����ʼ����ܽ�
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

	//���ý�ɫ�ƶ�
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately(); 
	if (BlasterPlayerController)
	{
		bDisableGameplay = true; //���ý�ɫ��������
		if (Combat)
		{
			Combat->FireButtonPressed(false); //���ÿ���
		}
	}

	//�ر���ײ
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//������̭������
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
		Combat->EquippedWeapon->Destroy(); //��������ѽ���������������
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


// ��֡���ã�Tick)
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);	//��ת��ɫ
	

	HideCameraIfChanracterColse();
	PollInit();
}

void APlayerCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay) 
	{
		bUseControllerRotationYaw = false;	
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;	//����ת��
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

// �����Խ����ܰ󶨵�����
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//��Ծ
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APlayerCharacter::Jump);

	//������
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);	//ǰ���ƶ�
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);		//�����ƶ�
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);					//����ת��
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);				//���¸���
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &APlayerCharacter::EquipButtonPressed);	//װ������
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::CrouchButtonPressed);	//����
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::AimButtonPressed);		//������׼
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::AimButtonReleased);	//�ɿ���׼
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerCharacter::FireButtonPressed);		//���¿���
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &APlayerCharacter::FireButtonReleased);	//�ɿ�����
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::ReloadButtonPressed);	//���»���
}

//��ʼ��������
void APlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

//���𶯻���̫��
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

//����������̫��
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

//��ɫǰ���ƶ�
void APlayerCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);		//��ȡƫ����
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) );	//����ƫ���Ǵ�����X��ķ�������
		AddMovementInput(Direction, Value);												//�ý�ɫ��X���ƶ���ǰ�����ˣ�
	}
}

//��ɫ�����ƶ�
void APlayerCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);		//��ȡƫ����
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));	//����ƫ���Ǵ�����Y��ķ�������
		AddMovementInput(Direction, Value);												//�ý�ɫ��Y���ƶ��������ƶ���
	}
}

void APlayerCharacter::Turn(float Value)	//ת��
{
	AddControllerYawInput(Value);
}

void APlayerCharacter::LookUp(float Value)	//�����ӽ�
{
	AddControllerPitchInput(Value);
}

void APlayerCharacter::EquipButtonPressed()	//ʰȡ����
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

void APlayerCharacter::CrouchButtonPressed()//����վ��
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

void APlayerCharacter::AimButtonPressed()//������׼
{
	if (bDisableGameplay) return;
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void APlayerCharacter::AimButtonReleased()//�ɿ���׼
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

void APlayerCharacter::AimOffset(float DeltaTime)//��׼ƫ��
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	//��ȡ�ٶ�
	float Speed = CalculateSpeed();

	//����Ƿ�����Ծ״̬
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	//����վ��״̬����û����Ծ��Ϊ
	if (Speed == 0.f && !bIsInAir)	
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation); //ƫ��������
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	
	//�ƶ�����Ծ
	if (Speed > 0.f || bIsInAir)	
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);	//���ý�ɫ�ĳ���ʹ��ɫ����׼�ķ���
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}
	
	CalculateAO_Ptich();
}

//����AO_Pitch
void APlayerCharacter::CalculateAO_Ptich()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//��pitch��ֵ��[270, 360)ӳ�䵽[-90, 0)
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

void APlayerCharacter::Jump()//��Ծ
{
	if (bDisableGameplay) return;
	if (bIsCrouched)//����ʱ������Ծ��վ��
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void APlayerCharacter::FireButtonPressed()	//���¿���ťʱ����
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


//��RPC)����˴���ʰȡ����
void APlayerCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

//����ת��״̬
void APlayerCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)//��ת״̬
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)//��ת״̬
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)	//������ת���״̬ʱ����ת��ɫ�ĸ������Ӷ��ý�ɫ�ܹ�������ȷ�ķ���
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);	//ƽ��������ת��
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)	//ƫ����С��15��ʱ���ý�ɫ��ת��״̬�����ý�ɫ�ĳ���
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





//���ڱ��ؽ�ɫ����ʾ"ʰȡ����"��UI�����ڽ������˲���ʾ�����⣩
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

//���縴���ص�������
void APlayerCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);	//��ʾ��ʰȡ��������UI
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//�Ƿ�װ��������
bool APlayerCharacter::IsWeaponEquipped()
{

	return (Combat && Combat->EquippedWeapon);
}

//�Ƿ�����׼״̬
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




