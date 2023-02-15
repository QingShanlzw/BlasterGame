// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	if(!HasAuthority())return;
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetMesh()->GetSocketByName(FName("MuzzleFlash"));
	UWorld* World = GetWorld();
	if (MuzzleFlashSocket && World)
	{
		UE_LOG(LogTemp,Warning,TEXT("MuzzleFlashSocket && World  is not nullptr"));
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetMesh());
		// From muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if(ProjectileClass&&InstigatorPawn)
		{
			UE_LOG(LogTemp,Warning,TEXT("ProjectileClass&&InstigatorPawn is not nullptr"));
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UE_LOG(LogTemp,Warning,TEXT("Spawn Actor!!"));
			World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParams
			);
		}

	}	
}
