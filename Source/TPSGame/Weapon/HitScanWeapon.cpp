// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return; //���û��ӵ���ߣ��򷵻�
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket && InstigatorController)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		FVector End = Start + (HitTarget - Start) * 1.25f;

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECollisionChannel::ECC_Visibility
			);
			if (FireHit.bBlockingHit)	//����˺�
			{
				APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
				if (PlayerCharacter)
				{
					if (HasAuthority())
					{
						UGameplayStatics::ApplyDamage(
						PlayerCharacter,				// ���˵Ľ�ɫ
						Damage,						// �˺�ֵ
						InstigatorController,	// ����˺��Ŀ�����
						this,						// ����˺�������
						UDamageType::StaticClass()	// ʹ��Ĭ�ϵ��˺�����
					);
					}
					
				}
				if (ImpactParticles)	//������������Ч��
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticles,
						FireHit.ImpactPoint,	//����λ��
						FireHit.ImpactNormal.Rotation(),	//���з��ߵ���ת
						FVector(1.f),			//����
						true					//�Ƿ��Զ�����
					);
				}
			}
		}
	}
	
	

	
		
}
