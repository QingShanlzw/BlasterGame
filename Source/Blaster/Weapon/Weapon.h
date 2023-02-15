// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	void ShowPickupWidget(bool bShowWidget);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual  void Fire( const FVector& HitTarget);
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	//五个准星
	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	virtual void Dropped();


	/** 
	* Zoomed FOV while aiming
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	//子弹数目

	UPROPERTY(EditAnywhere,ReplicatedUsing=OnRep_Ammo)
	int32 Ammo;
	UFUNCTION()
	void OnRep_Ammo();
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	void SpendRound();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);
	
	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//连击
	UPROPERTY(EditAnywhere,Category=Combat)
	float FireDelay= 0.15f;
	UPROPERTY(EditAnywhere,Category=Combat)
	bool bAutomatic =true;

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;
	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	

public:
	 void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere()const {return AreaSphere;}
	FORCEINLINE USkeletalMeshComponent* GetMesh()const {return WeaponMesh;}
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	
};
