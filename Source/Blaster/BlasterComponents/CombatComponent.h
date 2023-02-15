// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


#define TRACE_LENGTH 8000
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ABlasterCharacter;
	void EquipWeapon(class AWeapon* WeaponToEquip);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void Fire();
	void FireButtonPressed(bool bPressed);
	UFUNCTION(Server,Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast,Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void SetAiming(bool IsAiming);
	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool IsAiming);
	UFUNCTION()
	void OnRep_EquippedWeapon();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);
	
private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing= OnRep_EquippedWeapon)
	 AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;
	bool bFireButtonPressed;

	/** 
	* HUD and crosshairs
	*/

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FHUDPackage HUDPackage;

	/** 
	* Aiming and FOV
	*/

	// Field of view when not aiming; set to the camera's base FOV in BeginPlay
	float DefaultFOV;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;
	float CurrentFOV;
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	//自动开火：
	FTimerHandle FireTimer;
	
	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	

public:	
	
		
};
