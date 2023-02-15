// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{

	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor,Damage,OwnerController,this,UDamageType::StaticClass());
		}
	}
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
	
}
