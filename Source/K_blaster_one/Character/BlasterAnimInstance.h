// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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
};
