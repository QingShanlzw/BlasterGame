// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;

	FVector Velocity = BlasterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();
	
	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped =BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon =BlasterCharacter->GetEquippedWeapon();

	bIsCrouch = BlasterCharacter->bIsCrouched;

	bAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	
	bElimed =BlasterCharacter->IsElimmed();
	
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	// UE_LOG(LogTemp,Warning,TEXT("Blaster's Rotation is : %f,%f,%f"),BlasterCharacter->GetBaseAimRotation().Pitch,
	// BlasterCharacter->GetBaseAimRotation().Yaw,
	// BlasterCharacter->GetBaseAimRotation().Roll
	// 		);
	// UE_LOG(LogTemp,Warning,TEXT("Blaster's Rotation is : %f,%f,%f"),
	// BlasterCharacter->GetVelocity().X,
	// BlasterCharacter->GetVelocity().Y,
	// BlasterCharacter->GetVelocity().Z);
	//UE_LOG(LogTemp,Warning,TEXT("%f"),UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity()).Yaw);

	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();//镜头的偏移
	FRotator MovementRotation  = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());//移动的偏
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation).Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	
	AO_Pitch =BlasterCharacter->GetAO_Pitch();
	AO_Yaw =BlasterCharacter->GetAO_Yaw();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		//把插槽的translation转化为相对右手的骨骼空间
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}

