// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "K_blaster_one/HUD/BlasterHUD.h"
#include "K_blaster_one/Weapon/WeaponType.h"
#include "K_blaster_one/BlasterType/CombatState.h"
#include "CombatComponent.generated.h"
#define STEP 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class K_BLASTER_ONE_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	friend class ABlasterCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// 在该函数中注册要复制的变量
	
	void EquipWeapon(class AWeaponBase* WeaponToEquipped);

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
protected:
	virtual void BeginPlay() override;

	void SetAmingState(bool bAming);

	UFUNCTION(Server, Reliable)
	void ServerSetAming(bool bAming);

	UFUNCTION()
	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();
private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	UPROPERTY()
	ABlasterHUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeaponBase* EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	// HUD Crosshairs
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	
	FVector_NetQuantize HitTarget;
	
	FHUDPackage HUDPackage;
	
	// 没瞄准时候的视野大小
	float DefaultFOV;

	float CurrentFOV;
	
	UPROPERTY(EditAnywhere, Category = K_Combat)
	float ZoomedFOV = 30.f;
	
	UPROPERTY(EditAnywhere, Category = K_Combat)
	float ZoomInterpSpeed = 20.f;
	
	void InterpFOV(float DeltaTime);

	bool CanFire();
	
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;
	
	UFUNCTION()
	void OnRep_CarriedAmmo();
	
	TMap<EWeaponType, int32> CarriedAmmoMap;
	
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;
	
	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
public:	

		
};
