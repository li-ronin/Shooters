#include "BlasterCharacter.h"

//------------------------------
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "K_blaster_one/Components/CombatComponent.h"
#include "K_blaster_one/Weapon/WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "K_blaster_one/PlayerController/BlasterPlayerController.h"
#include "K_blaster_one/K_blaster_one.h"
#include "K_blaster_one/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "K_blaster_one/PlayerState/BlasterPlayerState.h"
#include "K_blaster_one/Weapon/WeaponType.h"

ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	// 摄像机弹簧臂
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600;
	CameraBoom->bUsePawnControlRotation = true;
	// 摄像机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;	// 让角色朝向移动方向，而不是镜头旋转方向

	// Combat组件，可装备武器
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));	
	Combat->SetIsReplicated(true);	  // 指定战斗组件为复制组件，复制Component不需要注册
	// // Combat->SetComponentTickEnabled(true);
	Combat->Character = this;
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Character 转身的速度
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	
	TurningState = ETurningInPlace::ETIP_NotTurning;

	// 网络更新频率
	/** How often (per second) this actor will be considered for replication, used to determine NetUpdateTime */
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	// Server Net Tick Rate在配置文件里面设置

	// 
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// 注册重叠武器的变量以复制,只有在拥有此角色的客户端上（也就是Remote=ROLE_AutonomousProxy）显示Widget才有意义，无需复制到所有客户端上。
	// DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
	BlasterPlayerController = Cast<ABlasterPlayerController>(Controller);
	UpdateHealth();
	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();
	if(ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}else
	{
		TimeSinceLastMovementReplication+=DeltaTime;
		if(TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
	HideCharacter();
	PollInit();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ThisClass::Jump);	// IE_Pressed是InputEventType
	// 绑定我们自定义的Action函数
	PlayerInputComponent->BindAxis("MoveForward", this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
	
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ThisClass::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ThisClass::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ThisClass::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ThisClass::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ThisClass::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ThisClass::FireButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ThisClass::ReloadButtonPressed);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimProxyTurn();		// 除了代理的Movement被复制的时候调用SimProxyTurn，我们还想定时的去调用它
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::Elim()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Dropped();	
	}
	
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ThisClass::ElimTimerFinished,
		ElimDelay
		);
}

void ABlasterCharacter::MulticastElim_Implementation()
{
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();
	// 身体消失效果
	if(DissolveMaterial)
	{
		DynamicDissolveMaterial = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterial);
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Glow"), 200.f);
		StartDissolve();
	}
	// Disable Player Movement
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if(BlasterPlayerController)
	{
		DisableInput(BlasterPlayerController);
	}
	// Disable Player Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 机器人特效
	if(ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z+200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(this,
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
			);
	}
	// 机器人声音
	if(ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
}

void ABlasterCharacter::ElimTimerFinished()
{
	// Timer Finish Respawn Character
	ABlasterGameMode* BlasterGameMode =GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if(BlasterGameMode)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterial)
	{
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial); // DissolveTrack代理绑定回调函数
	if(DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack); // Timeline添加曲线和Delegate，一旦开始播放就会通知Delegate的回调函数
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if(!Combat || !Combat->EquippedWeapon)return;
	UAnimInstance* OurAnimInstance = GetMesh()->GetAnimInstance();
	if(OurAnimInstance && FireMontage)
	{
		OurAnimInstance->Montage_Play(FireMontage);
		FName SectionName = bAiming ? TEXT("RifleAim") : TEXT("RifleHip");
		OurAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if(!Combat || !Combat->EquippedWeapon)return;
	
	UAnimInstance* OurAnimInstance = GetMesh()->GetAnimInstance();
	if(OurAnimInstance && ReloadMontage)
	{
		OurAnimInstance->Montage_Play(ReloadMontage);
		FName SectionName ;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		}
		OurAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* OurAnimInstance = GetMesh()->GetAnimInstance();
	if(OurAnimInstance && ElimMontage)
	{
		OurAnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	UAnimInstance* OurAnimInstance = GetMesh()->GetAnimInstance();
	if(OurAnimInstance && HitReactMontage)
	{
		OurAnimInstance->Montage_Play(HitReactMontage);
		FName SectionName = TEXT("FromLeft");
		OurAnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health-Damage, 0.f, MaxHealth);
	UpdateHealth();
	PlayHitReactMontage();
	if(Health <= 0.f){
		ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
		if(BlasterGameMode)
		{
			BlasterPlayerController = BlasterPlayerController==nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::OnRep_Health()
{
	UpdateHealth();
	PlayHitReactMontage();
}

void ABlasterCharacter::UpdateHealth()
{
	BlasterPlayerController = (BlasterPlayerController==nullptr) ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;	
	if(BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

void ABlasterCharacter::PollInit()
{
	if(!BlasterPlayerState)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if(BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeat(0);
		}
	}
}

void ABlasterCharacter::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}else
	{
		Super::Jump();		
	}
}

void ABlasterCharacter::MoveForward(float Value)
{
	if(Controller!=nullptr && Value!=0.f)
	{
		// 获取前方方向
		// 这里不使用GetActorForwardVector，是因为我们要获取的是Controller的前向而不是Actor的
		const FRotator YawRotation{0.f,Controller->GetControlRotation().Yaw, 0.f};	// Yaw是左右旋转
		const FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) };	// 用YawRotation创建一个FRotationMatriX并且获取它的X单位轴，返回的Vector就是Controller的前向
		// 移动
		AddMovementInput(Direction, Value);	// Value = 1向前， Value=-1向后
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if(Controller!=nullptr && Value!=0.f)
	{
		const FRotator YawRotation{0.f,Controller->GetControlRotation().Yaw, 0.f};	
		const FVector Direction{ FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) };	
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if(Combat)	// 仅在服务器上进行装备武器
	{
		if(GetLocalRole() == ROLE_Authority)
		{
			Combat->EquipWeapon(OverlappingWeapon);	
		}else
		{
			ServerEquipButtonPressed();
		}
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if(Combat)	// 客户端要想装备武器调用server的RPC来装备
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimButtonPressed()
{
	if(Combat)
	{
		Combat->SetAmingState(true); 
	}
}

void ABlasterCharacter::AimButtonReleased()
{
	if(Combat)
	{
		Combat->SetAmingState(false); 
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if(Combat && Combat->EquippedWeapon)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::ReloadButtonPressed()
{
	if(Combat)
	{
		Combat->Reload();
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size(); // 模长Sqrt(X*X + Y*Y + Z*Z);
}
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if(Combat && !Combat->EquippedWeapon)return; 
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if(FMath::Abs(Speed) <= 1e-6 && !bIsInAir)	// 站着不动，并且没有跳跃
	{
		bRotateRootBone = true;
		bUseControllerRotationYaw = true;
		FRotator CurrAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); 
		AimOffset_Yaw = UKismetMathLibrary::NormalizedDeltaRotator(CurrAimRotation, StartingAimRotation).Yaw;
		if(TurningState == ETurningInPlace::ETIP_NotTurning){
			Last_Yaw = AimOffset_Yaw;
		}
		SetTurnInPlace(DeltaTime);
	}else
	{
		bRotateRootBone = false;
		bUseControllerRotationYaw = true;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AimOffset_Yaw = 0.f;
		TurningState = ETurningInPlace::ETIP_NotTurning;
	}
	// [90, 0] [0, -90]
	// [90, 0] [360, 270]
	// 在客户端向服务端发RPC的时候，CharacterMovmentComponent将旋转压缩至5Bytes
	// 压缩的时候将Yaw和Pitch [0, 360)-->[0, 65536)
	// 注意都是正数，所以当负数映射的时候，由于做了Mask会切断不在[0, 360)范围内的数
	// 当在服务端对客户端的角度进行解压缩的时候[0, 65536)-->[0, 360)，这就是[0, -90]的角度变成了[360, 270]的原因
	CalculateAO_Pitch();
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AimOffset_Pitch = GetBaseAimRotation().Pitch;
	
	if(AimOffset_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pithc from [270, 360) to [-90, 0)
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		AimOffset_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimOffset_Pitch);
	}
}

void ABlasterCharacter::SimProxyTurn()
{
	if(!Combat || !Combat->EquippedWeapon)return;
	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	if(Speed > 1e-6 || bIsInAir)
	{
		TurningState = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningState =  ETurningInPlace::ETIP_Right;
		}else if(ProxyYaw < -TurnThreshold)
		{
			TurningState =  ETurningInPlace::ETIP_Left;
		}else
		{
			TurningState = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningState = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::SetTurnInPlace(float DeltaTime)
{
	if(AimOffset_Yaw > 90.f)
	{
		TurningState = ETurningInPlace::ETIP_Right;
	}else if(AimOffset_Yaw < -90.f)
	{
		TurningState = ETurningInPlace::ETIP_Left;
	}
	if(TurningState != ETurningInPlace::ETIP_NotTurning)
	{
		Last_Yaw = FMath::FInterpTo(Last_Yaw, 0.f, DeltaTime, 4.f);
		AimOffset_Yaw = Last_Yaw;
		if(FMath::Abs(AimOffset_Yaw) < 15.f)
		{
			TurningState = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCharacter()
{
	if(!IsLocallyControlled())return;
	if((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraDistance)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::SetOverlapWeapon(AWeaponBase* weapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = weapon;
	if(IsLocallyControlled())	// 如果是服务器控制的Pawn发生了重叠单独处理,服务器本地控制的Character就不会将变量复制给客户端
	{
		if(OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
		
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeaponBase* LastWeapon)
{
	if(OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if(LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (Combat && Combat->bIsAiming);
}

AWeaponBase* ABlasterCharacter::GetEquippedWeapon()
{
	if(Combat && Combat->EquippedWeapon)
	{
		return Combat->EquippedWeapon;
	}
	return nullptr;
}

FVector_NetQuantize ABlasterCharacter::GetHitTarget()
{
	if(!Combat)return  FVector();
	return Combat->HitTarget;
}





