// Fill out your copyright notice in the Description page of Project Settings.

#include "CombatComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "K_blaster_one/Character/BlasterCharacter.h"
#include "K_blaster_one/Weapon/WeaponBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BaseWalkSpeed = 350.f;
	AimWalkSpeed = 150.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if(Character){
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 服务器复制给客户端的变量复制
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);   
	DOREPLIFETIME(UCombatComponent, bIsAiming);
}

void UCombatComponent::OnRep_EquippedWeapon()	// 复制给客户端时的处理
{
	if(EquippedWeapon && Character)
	{
		//  这是客户端的Character
		Character->bUseControllerRotationYaw = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;	// 拿到武器之后让角色朝向镜头旋转方向, 而不是移动方向
	}
}

void UCombatComponent::SetAmingState(bool bAming)
{
	bIsAiming = bAming;		// 本地上我们角色的bIsAiming变量
	ServerSetAming(bAming);	// 服务器上我们角色的bIsAiming变量。ServerSetAming可能从客户端被调用，也可能从服务端被调用
	if(Character){
		Character->GetCharacterMovement()->MaxWalkSpeed = bAming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAming_Implementation(bool bAming)
{
	bIsAiming = bAming;
	if(Character){
		Character->GetCharacterMovement()->MaxWalkSpeed = bAming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AWeaponBase* WeaponToEquipped)
{
	if(!Character || !WeaponToEquipped) return;	
	EquippedWeapon = WeaponToEquipped;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	
	Character->bUseControllerRotationYaw = true;
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;	// 拿到武器之后让角色朝向镜头旋转方向, 而不是移动方向
	
}


