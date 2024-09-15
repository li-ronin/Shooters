// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "K_blaster_one/BlasterType/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class K_BLASTER_ONE_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	void NativeInitializeAnimation() override;
	void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = K_Character, meta = (AllowPrivateAccess="true"))	// 私有变量需要设置AllowPrivateAccess="true"
	class ABlasterCharacter* OurCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	float Speed;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	bool bIsAccelerating;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	bool bWeaponEquipped;

	class AWeaponBase* EquippedWeapon;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	float YawOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	float Lean;

	FRotator CharacterRotatorLastFrame;
	FRotator CharacterRotatorCurrFrame;
	FRotator DeltaRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	float AO_Pitch;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Movement, meta = (AllowPrivateAccess="true"))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	FTransform LeftHandTransform;
	
	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = K_Combat, meta = (AllowPrivateAccess="true"))
	bool bElimmed;
	
};
