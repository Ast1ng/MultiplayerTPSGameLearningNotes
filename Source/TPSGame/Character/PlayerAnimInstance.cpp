// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"
#include "PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TPSGame/Weapon/Weapon.h"
#include "TPSGame/PlayerTypes/CombatState.h"

void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	//初始化角色
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
}

//每tick更新的内容
void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PlayerCharacter == nullptr)
	{
		//初始化角色
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if (PlayerCharacter == nullptr) return;


	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	//
	//初始化参数
	//
	bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();
	EquippedWeapon = PlayerCharacter->GetEquippedWeapon();
	bIsCrouched = PlayerCharacter->bIsCrouched;
	bAiming = PlayerCharacter->IsAiming();
	TurningInPlace = PlayerCharacter->GetTurningInPlace();
	bRotateRootBone = PlayerCharacter->ShouldRotateRootBone();
	bElimmed = PlayerCharacter->IsElimmed();

	//offset Yaw for Strafing 调整角色的偏航角
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();	//摄像机朝向的偏航角（全局坐标系）
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());	//角色移动方向的偏航角（全局坐标系）
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);	//获取偏航角的增量并归一化
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 3.f);	//对增量进行插值得到一个平滑的转向过渡
	YawOffset = DeltaRotation.Yaw;

	//计算角色旋转角速度并平滑插值，生成一个用于动画混合空间的 Lean 参数
	CharacterRotationLastFrame = CharacterRotation;				//上一帧的角度
	CharacterRotation = PlayerCharacter->GetActorRotation();	//当前帧的角度
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);	// 计算两帧之间的旋转差值（归一化到 [-180°, 180°]）
	const float Target = Delta.Yaw / DeltaTime;	// 计算目标角速度（Yaw 轴的旋转速度）
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 3.f);	// 对 Lean 值进行插值，平滑过渡到目标角速度
	Lean = FMath::Clamp(Interp, -90.f, 90.f);	// 将 Lean 值限制在 [-90°, 90°] 范围内
	
	AO_Yaw = PlayerCharacter->GetAO_Yaw();	//获取角色偏航角（用于瞄准偏移）
	AO_Pitch = PlayerCharacter->GetAO_Pitch();	//获取角色俯仰角（用于瞄准偏移）

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PlayerCharacter->GetMesh()->TransformToBoneSpace(FName("Wrist_R"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (PlayerCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			//旋转手部骨骼使枪口朝向准星
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Wrist_R"), ERelativeTransformSpace::RTS_World);	//获取右手的变换
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), PlayerCharacter->GetHitTarget());	//计算从手部骨骼到命中目标点之间的旋转角
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
	}

	bUseFABRIK = PlayerCharacter->GetCombatState() != ECombatState::ECS_Reloading;
	bUseAimOffsets = PlayerCharacter->GetCombatState() != ECombatState::ECS_Reloading && !PlayerCharacter->GetDIsableGameplay();
	bTransformRightHand = PlayerCharacter->GetCombatState() != ECombatState::ECS_Reloading && !PlayerCharacter->GetDIsableGameplay();
}
