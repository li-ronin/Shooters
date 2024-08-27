// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "K_blaster_one/BlasterType/TurningInPlace.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class K_BLASTER_ONE_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// 在该函数中注册要复制的变量

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	
	void PlayHitReactMontage();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	// Action Functions
	// void Jump();
	virtual void Jump() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void FireButtonPressed();
	void FireButtonReleased();
	
	void AimOffset(float DeltaTime);
private:
	UPROPERTY(VisibleAnywhere, Category = K_Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = K_Camera)
	class UCameraComponent* FollowCamera;
	
	// 复制变量，从服务器上复制过来的 UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeaponBase* OverlappingWeapon;
	
	// 这是一个Rep Notify，当变量被复制时负责通知，约定的函数名“OnRep_被复制的变量名称”
	// 复制通知，也就是没有复制就不会通知
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeaponBase* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();	// 客户端“按下E捡起武器时”调用服务端的RPC来执行

	float AimOffset_Yaw;
	float Last_Yaw;
	float AimOffset_Pitch;

	FRotator StartingAimRotation;

	ETurningInPlace TurningState;
	
	void SetTurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = K_Combat)
	class UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = K_Combat)
	class UAnimMontage* HitReactMontage;
	
	void Hide();

	UPROPERTY(EditAnywhere)
	float CameraDistance = 150.f;
	
public:
	// 由于重叠的检测只在服务器上，所以客户端上要想要显示重叠就需要把服务器的变量值复制给客户端。
	// 一旦角色和武器重叠时，就把OverlappingWeapon变量复制到所有客户端的角色上，复制只在变量改变的时候起作用，并不会每帧都更新
	// 在Weapon类的重叠函数中调用该函数把重叠的武器复制给客户端
	void SetOverlapWeapon(AWeaponBase* weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() { return AimOffset_Yaw; } 
	FORCEINLINE float GetAO_Pitch() { return AimOffset_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningState() { return TurningState; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const {return FollowCamera;}
	AWeaponBase* GetEquippedWeapon();
	FVector_NetQuantize GetHitTarget();
};

