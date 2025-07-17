// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TPSGame/Character/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return; //���û��ӵ���ߣ��򷵻�
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		TMap<APlayerCharacter*, int32> HitMap; //��¼������Һ����д���
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(FireHit.GetActor());
			if (PlayerCharacter && HasAuthority() && InstigatorController)
			{
				//�����������Ѿ�������HitMap�У����������д�������������µ���Һ����д���Ϊ1
				if (HitMap.Contains(PlayerCharacter))
				{
					HitMap[PlayerCharacter]++;
				}
				else
				{
					HitMap.Emplace(PlayerCharacter, 1);
				}
			}

			if (ImpactParticles)	//������������Ч��
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					this,
					ImpactParticles,
					FireHit.ImpactPoint,	//����λ��
					FireHit.ImpactNormal.Rotation(),	//���з��ߵ���ת
					FVector(1.f),			//����
					true					//�Ƿ��Զ�����
				);
			}
			if (HitSound) //ֻ����û������������Ч������������Ч��ʱ�����Ҫ��������Ч
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,	//����λ��
					.5f, //����
					FMath::FRandRange(-0.5f, 0.5f) //�������ƫ����
				);
			}
		}
		for (auto HitPair : HitMap)
		{
			if (InstigatorController)
			{
				//��ÿ�����е����Ӧ���˺�
				if (HitPair.Key && HasAuthority() && InstigatorController)
				{

					UGameplayStatics::ApplyDamage(
						HitPair.Key,				// ���˵Ľ�ɫ
						Damage * HitPair.Value,		// �˺�ֵ
						InstigatorController,		// ����˺��Ŀ�����
						this,						// ����˺�������
						UDamageType::StaticClass()	// ʹ��Ĭ�ϵ��˺�����
					);
				}
			}
		}
	
	}
}

