#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "K_blaster_one/Weapon/WeaponBase.h"


void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	OurCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	
}

// 每帧调用
void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(OurCharacter == nullptr)
	{
		OurCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if(OurCharacter == nullptr)return;

	FVector Velocity = OurCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size(); // 模长Sqrt(X*X + Y*Y + Z*Z);
	
	bIsInAir = OurCharacter->GetCharacterMovement()->IsFalling();
	
	bIsAccelerating = OurCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

	bIsCrouched = OurCharacter->bIsCrouched;
	
	bWeaponEquipped = OurCharacter->IsWeaponEquipped();
	
	EquippedWeapon = OurCharacter->GetEquippedWeapon();
	
	bIsAiming = OurCharacter->IsAiming();
	
	TurningInPlace = OurCharacter->GetTurningState();

	bRotateRootBone = OurCharacter->ShouldRotateRootBone();

	bElimmed = OurCharacter->IsElimmed();
	// Yaw和Lean 当移动时的方向角度，用来控制播放哪个移动动画（Blend space）
	// 偏航角为角色的前进方向和瞄准方向的差,客户端和服务器上都有该变量无须担心复制
	FRotator AimRotation = OurCharacter->GetBaseAimRotation();	// 世界的东南西北偏航角
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(OurCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 5.f);
	YawOffset = DeltaRotation.Yaw;
	
	CharacterRotatorLastFrame = CharacterRotatorCurrFrame;
	CharacterRotatorCurrFrame = OurCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotatorCurrFrame, CharacterRotatorLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp,-90.f, 90.f);

	// 原地不同时，可旋转上半身的角度
	AO_Yaw = OurCharacter->GetAO_Yaw();
	AO_Pitch = OurCharacter->GetAO_Pitch();

	if(	bWeaponEquipped && EquippedWeapon )
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), RTS_World);
		// 左手放在武器应该位移和旋转的量（Bone Space）
		FVector OutPosition;
		FRotator OutRotation;
		// 取右手骨骼的tranform来做为骨骼空间的参照系
		OurCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));		// 四元数
		// LeftHandTransform是BoneSpace中左手的正确位置变换，具体的IK变换在AnimBP中完成的
		if(OurCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"), RTS_World);
			// 旋转右手骨骼，让拿枪的时候枪口指向瞄准的目标
			// 获得从当前角度到目标角度的旋转
			FRotator LookatRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation()+(RightHandTransform.GetLocation()-OurCharacter->GetHitTarget())); 
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookatRotation, DeltaSeconds, 20.f);
		}
		// FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), RTS_World);
		// FVector Muzzle_X(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		// DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + Muzzle_X*100.f, FColor::Red);
		// DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(),OurCharacter->GetHitTarget() , FColor::Blue);
	}
	
}
