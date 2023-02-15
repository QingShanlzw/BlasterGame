// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "DrawDebugHelpers.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/Weapon/Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...

	BaseWalkSpeed =600.f;
	AimWalkSpeed =450.f;
}




// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed =BaseWalkSpeed;
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		
	}
	bCanFire =true;
}

void UCombatComponent::SetAiming(bool IsAiming)
{
	ServerSetAiming(IsAiming);
}



void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		//Debug
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint=End;
		}
		//HitTarget =TraceHitResult.ImpactPoint;
		DrawDebugSphere(
			GetWorld(),
			TraceHitResult.ImpactPoint,
			12.f,
			12,
			FColor::Red
		);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}
		
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			
			if (EquippedWeapon)
			{
				//FHUDPackage HUDPackage;
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				//HUD->SetHUDPackage(HUDPackage);
			}
			else
			{
				//FHUDPackage HUDPackage;
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				//HUD->SetHUDPackage(HUDPackage);
				
			}
			// Calculate crosshair spread

			// [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
			HUDPackage.CrosshairSpread =CrosshairVelocityFactor;
			
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}
			if (bIsAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}	
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bIsAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire =true;
	if(bFireButtonPressed && EquippedWeapon&&EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}
void UCombatComponent::Fire()
{
	if(bCanFire)
	{
		UE_LOG(LogTemp,Warning,TEXT("Can Fire."));
		bCanFire = false;
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
		if(EquippedWeapon)
		{
			CrosshairShootingFactor = 0.2f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool IsAiming)
{
	bIsAiming = IsAiming;
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	
	if(Character && Character->IsLocallyControlled())
	{
		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
	
}
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent,EquippedWeapon);
	DOREPLIFETIME(UCombatComponent,bIsAiming);
}



void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if(bFireButtonPressed)
	{
		bCanFire =true;
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon==nullptr)return;
	if(Character)
	{
		Character->PlayFireMontage(bFireButtonPressed);
		EquippedWeapon ->Fire(TraceHitTarget);
	}
}

//装备武器
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if(Character == nullptr || WeaponToEquip == nullptr)
	{
		return;
	}
	if(EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	EquippedWeapon = WeaponToEquip;
	//EquippedWeapon->ShowPickupWidget(false);
	//EquippedWeapon->GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket  =Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
	}
	//SetOwner主要用于网络复制.
	EquippedWeapon->SetOwner(Character);
	//主要用于刚刚捡起武器，server更新那个HUD的弹药数（client的在setOwner的时候，通过网络通知函数调用了）
	EquippedWeapon->SetHUDAmmo();
	
	Character->GetCharacterMovement()->bOrientRotationToMovement =false;
	Character->bUseControllerRotationYaw =true;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if(EquippedWeapon &&Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket  =Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon,Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement =false;
		Character->bUseControllerRotationYaw =true;
		
	}
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming? AimWalkSpeed:BaseWalkSpeed;
	}
}

