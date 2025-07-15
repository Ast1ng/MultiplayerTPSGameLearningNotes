// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	
	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());	// ��ȡ����������Pawn

	// �����������ȡǹ��������(�䵯���ɵ�λ��)
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	
	
	if (MuzzleFlashSocket)
	{
		// ��ȡǹ�ڲ�۵�Transform��λ��/��ת/���ţ�
		FTransform SocketTransform =  MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		
		
		 // �����ǹ�ڵ�Ŀ��������������Ŀ����ת
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();	//�����䵯��ӵ����
			SpawnParams.Instigator = InstigatorPawn;	//���ù����ķ�����Ϊ����������pawn
			UWorld* World = GetWorld();
			
			if (World)
			{
				// �����䵯Actor
				World->SpawnActor<AProjectile>(
					ProjectileClass,				// �䵯��
					SocketTransform.GetLocation(),	// ����λ�ã�ǹ�ڣ�
					TargetRotation,					// �䵯����
					SpawnParams						// ���ɲ���
				);
			}
		}
	}
}
