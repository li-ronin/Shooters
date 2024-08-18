// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Init UMETA(DisplayName = "Initial"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class K_BLASTER_ONE_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponBase();

	virtual void Tick(float DeltaTime) override;

	void ShowPickupWidget(bool bShowWidget);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	// 在该函数中注册要复制的变量
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComponent,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	
private:
	UPROPERTY(VisibleAnywhere, Category=WeaponProperty)
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category=WeaponProperty)
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing=OnRep_WeaponState, VisibleAnywhere, Category=WeaponProperty)
	EWeaponState WeaponState;

	// 这是一个Rep Notify，当变量被复制时负责通知，约定的函数名“OnRep_被复制的变量名称”
	UFUNCTION()
	void OnRep_WeaponState();
	
	UPROPERTY(VisibleAnywhere, Category=WeaponProperty)
	class UWidgetComponent* PickupWidget;
public:
	void SetWeaponState(EWeaponState state);
	FORCEINLINE EWeaponState GetWeaponState(){return WeaponState;}
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh(){return WeaponMesh;}
	FORCEINLINE USphereComponent* GetAreaSphere() const{return AreaSphere;}
	
	
};
